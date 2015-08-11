// Module for mqtt

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

#include "c_string.h"
#include "c_stdlib.h"

#include "c_types.h"
#include "mem.h"
#include "espconn.h"

#include "mqtt_msg.h"
#include "msg_queue.h"

#define MQTT_BUF_SIZE 1024
#define MQTT_DEFAULT_KEEPALIVE 60
#define MQTT_MAX_CLIENT_LEN   64
#define MQTT_MAX_USER_LEN     64
#define MQTT_MAX_PASS_LEN     64
#define MQTT_SEND_TIMEOUT			5
#define MQTT_CONNECT_TIMEOUT  5

typedef enum {
  MQTT_INIT,
  MQTT_CONNECT_SENT,
  MQTT_CONNECT_SENDING,
  MQTT_DATA
} tConnState;

typedef struct mqtt_event_data_t
{
  uint8_t type;
  const char* topic;
  const char* data;
  uint16_t topic_length;
  uint16_t data_length;
  uint16_t data_offset;
} mqtt_event_data_t;

typedef struct mqtt_state_t
{
  uint16_t port;
  int auto_reconnect;
  mqtt_connect_info_t* connect_info;
  uint16_t message_length;
  uint16_t message_length_read;
  mqtt_connection_t mqtt_connection;
  msg_queue_t* pending_msg_q;
} mqtt_state_t;

typedef struct lmqtt_userdata
{
  lua_State *L;
  struct espconn *pesp_conn;
  int self_ref;
  int cb_connect_ref;
  int cb_disconnect_ref;
  int cb_message_ref;
  int cb_suback_ref;
  int cb_puback_ref;
  mqtt_state_t  mqtt_state;
  mqtt_connect_info_t connect_info;
  uint32_t keep_alive_tick;
  uint32_t event_timeout;
  uint8_t secure;
  bool connected;     // indicate socket connected, not mqtt prot connected.
  ETSTimer mqttTimer;
  tConnState connState;
}lmqtt_userdata;

static void socket_connect(struct espconn *pesp_conn);
static void mqtt_socket_reconnected(void *arg, sint8_t err);
static void mqtt_socket_connected(void *arg);

static void mqtt_socket_disconnected(void *arg)    // tcp only
{
  NODE_DBG("enter mqtt_socket_disconnected.\n");
  struct espconn *pesp_conn = arg;
  bool call_back = false;
  if(pesp_conn == NULL)
    return;
  lmqtt_userdata *mud = (lmqtt_userdata *)pesp_conn->reverse;
  if(mud == NULL)
    return;

  os_timer_disarm(&mud->mqttTimer);

  if(mud->connected){     // call back only called when socket is from connection to disconnection.
    mud->connected = false;
    if((mud->L != NULL) && (mud->cb_disconnect_ref != LUA_NOREF) && (mud->self_ref != LUA_NOREF)) {
      lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->cb_disconnect_ref);
      lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->self_ref);  // pass the userdata(client) to callback func in lua
      call_back = true;
    }
  }

  if(mud->mqtt_state.auto_reconnect){
    mud->pesp_conn->reverse = mud;
    mud->pesp_conn->type = ESPCONN_TCP;
    mud->pesp_conn->state = ESPCONN_NONE;
    mud->connected = false;
    mud->pesp_conn->proto.tcp->remote_port = mud->mqtt_state.port;
    mud->pesp_conn->proto.tcp->local_port = espconn_port();
    espconn_regist_connectcb(mud->pesp_conn, mqtt_socket_connected);
    espconn_regist_reconcb(mud->pesp_conn, mqtt_socket_reconnected);
    socket_connect(pesp_conn);
  } else {
    if(mud->pesp_conn){
      mud->pesp_conn->reverse = NULL;
      if(mud->pesp_conn->proto.tcp)
        c_free(mud->pesp_conn->proto.tcp);
      mud->pesp_conn->proto.tcp = NULL;
      c_free(mud->pesp_conn);
      mud->pesp_conn = NULL;
    }

    if(mud->L == NULL)
      return;
    lua_gc(mud->L, LUA_GCSTOP, 0);
    if(mud->self_ref != LUA_NOREF){   // TODO: should we unref the client and delete it?
      luaL_unref(mud->L, LUA_REGISTRYINDEX, mud->self_ref);
      mud->self_ref = LUA_NOREF; // unref this, and the mqtt.socket userdata will delete it self
    }
    lua_gc(mud->L, LUA_GCRESTART, 0);
  }

  if((mud->L != NULL) && call_back){
    lua_call(mud->L, 1, 0);
  }

  NODE_DBG("leave mqtt_socket_disconnected.\n");
}

static void mqtt_socket_reconnected(void *arg, sint8_t err)
{
  NODE_DBG("enter mqtt_socket_reconnected.\n");
  // mqtt_socket_disconnected(arg);
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL)
    return;
  lmqtt_userdata *mud = (lmqtt_userdata *)pesp_conn->reverse;
  if(mud == NULL)
    return;

  os_timer_disarm(&mud->mqttTimer);

  if(mud->mqtt_state.auto_reconnect){
    pesp_conn->proto.tcp->remote_port = mud->mqtt_state.port;
    pesp_conn->proto.tcp->local_port = espconn_port();
    socket_connect(pesp_conn);
  } else {
    mqtt_socket_disconnected(arg);
  }
  NODE_DBG("leave mqtt_socket_reconnected.\n");
}

static void deliver_publish(lmqtt_userdata * mud, uint8_t* message, int length)
{
  NODE_DBG("enter deliver_publish.\n");
  if(mud == NULL)
    return;
  mqtt_event_data_t event_data;

  event_data.topic_length = length;
  event_data.topic = mqtt_get_publish_topic(message, &event_data.topic_length);

  event_data.data_length = length;
  event_data.data = mqtt_get_publish_data(message, &event_data.data_length);

  if(mud->cb_message_ref == LUA_NOREF)
    return;
  if(mud->self_ref == LUA_NOREF)
    return;
  if(mud->L == NULL)
    return;
  if(event_data.topic && (event_data.topic_length > 0)){
    lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->cb_message_ref);
    lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->self_ref);  // pass the userdata to callback func in lua
    lua_pushlstring(mud->L, event_data.topic, event_data.topic_length);
  } else {
    NODE_DBG("get wrong packet.\n");
    return;
  }
  if(event_data.data && (event_data.data_length > 0)){
    lua_pushlstring(mud->L, event_data.data, event_data.data_length);
    lua_call(mud->L, 3, 0);
  } else {
    lua_call(mud->L, 2, 0);
  }
  NODE_DBG("leave deliver_publish.\n");
}

static void mqtt_socket_received(void *arg, char *pdata, unsigned short len)
{
  NODE_DBG("enter mqtt_socket_received.\n");

  uint8_t msg_type;
  uint8_t msg_qos;
  uint16_t msg_id;
  msg_queue_t *node = NULL;
  int length = (int)len;
  // uint8_t in_buffer[MQTT_BUF_SIZE];
  uint8_t *in_buffer = (uint8_t *)pdata;

  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL)
    return;
  lmqtt_userdata *mud = (lmqtt_userdata *)pesp_conn->reverse;
  if(mud == NULL)
    return;

READPACKET:
  if(length > MQTT_BUF_SIZE || length <= 0)
	  return;

  // c_memcpy(in_buffer, pdata, length);
  uint8_t temp_buffer[MQTT_BUF_SIZE];
  mqtt_msg_init(&mud->mqtt_state.mqtt_connection, temp_buffer, MQTT_BUF_SIZE);
  mqtt_message_t *temp_msg = NULL;
  switch(mud->connState){
    case MQTT_CONNECT_SENDING:
    case MQTT_CONNECT_SENT:
      if(mqtt_get_type(in_buffer) != MQTT_MSG_TYPE_CONNACK){
        NODE_DBG("MQTT: Invalid packet\r\n");
        mud->connState = MQTT_INIT;
        if(mud->secure)
          espconn_secure_disconnect(pesp_conn);
        else
          espconn_disconnect(pesp_conn);
      } else {
        mud->connState = MQTT_DATA;
        NODE_DBG("MQTT: Connected\r\n");
        if(mud->cb_connect_ref == LUA_NOREF)
          break;
        if(mud->self_ref == LUA_NOREF)
          break;
        if(mud->L == NULL)
          break;
        lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->cb_connect_ref);
        lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->self_ref);  // pass the userdata(client) to callback func in lua
        lua_call(mud->L, 1, 0);
        break;
      }
      break;

    case MQTT_DATA:
      mud->mqtt_state.message_length_read = length;
      mud->mqtt_state.message_length = mqtt_get_total_length(in_buffer, mud->mqtt_state.message_length_read);
      msg_type = mqtt_get_type(in_buffer);
      msg_qos = mqtt_get_qos(in_buffer);
      msg_id = mqtt_get_id(in_buffer, mud->mqtt_state.message_length);

      msg_queue_t *pending_msg = msg_peek(&(mud->mqtt_state.pending_msg_q));

      NODE_DBG("MQTT_DATA: type: %d, qos: %d, msg_id: %d, pending_id: %d\r\n",
            msg_type,
            msg_qos,
            msg_id,
            (pending_msg)?pending_msg->msg_id:0);
      switch(msg_type)
      {
        case MQTT_MSG_TYPE_SUBACK:
          if(pending_msg && pending_msg->msg_type == MQTT_MSG_TYPE_SUBSCRIBE && pending_msg->msg_id == msg_id){
            NODE_DBG("MQTT: Subscribe successful\r\n");
            msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
            if (mud->cb_suback_ref == LUA_NOREF)
              break;
            if (mud->self_ref == LUA_NOREF)
              break;
            if(mud->L == NULL)
              break;
            lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->cb_suback_ref);
            lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->self_ref);
            lua_call(mud->L, 1, 0);
          }
          break;
        case MQTT_MSG_TYPE_UNSUBACK:
          if(pending_msg && pending_msg->msg_type == MQTT_MSG_TYPE_UNSUBSCRIBE && pending_msg->msg_id == msg_id){
            NODE_DBG("MQTT: UnSubscribe successful\r\n");
            msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
          }
          break;
        case MQTT_MSG_TYPE_PUBLISH:
          if(msg_qos == 1){
            temp_msg = mqtt_msg_puback(&mud->mqtt_state.mqtt_connection, msg_id);
            node = msg_enqueue(&(mud->mqtt_state.pending_msg_q), temp_msg, 
                      msg_id, MQTT_MSG_TYPE_PUBACK, (int)mqtt_get_qos(temp_msg->data) );
          }
          else if(msg_qos == 2){
            temp_msg = mqtt_msg_pubrec(&mud->mqtt_state.mqtt_connection, msg_id);
            node = msg_enqueue(&(mud->mqtt_state.pending_msg_q), temp_msg, 
                      msg_id, MQTT_MSG_TYPE_PUBREC, (int)mqtt_get_qos(temp_msg->data) );
          }
          if(msg_qos == 1 || msg_qos == 2){
            NODE_DBG("MQTT: Queue response QoS: %d\r\n", msg_qos);
          }
          deliver_publish(mud, in_buffer, mud->mqtt_state.message_length);
          break;
        case MQTT_MSG_TYPE_PUBACK:
          if(pending_msg && pending_msg->msg_type == MQTT_MSG_TYPE_PUBLISH && pending_msg->msg_id == msg_id){
            NODE_DBG("MQTT: Publish with QoS = 1 successful\r\n");
            msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
            if(mud->cb_puback_ref == LUA_NOREF)
              break;
            if(mud->self_ref == LUA_NOREF)
              break;
            if(mud->L == NULL)
              break;
            lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->cb_puback_ref);
            lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->self_ref);  // pass the userdata to callback func in lua
            lua_call(mud->L, 1, 0);
          }

          break;
        case MQTT_MSG_TYPE_PUBREC:
          if(pending_msg && pending_msg->msg_type == MQTT_MSG_TYPE_PUBLISH && pending_msg->msg_id == msg_id){
            NODE_DBG("MQTT: Publish  with QoS = 2 Received PUBREC\r\n"); 
            // Note: actrually, should not destroy the msg until PUBCOMP is received.
            msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
            temp_msg = mqtt_msg_pubrel(&mud->mqtt_state.mqtt_connection, msg_id);
            node = msg_enqueue(&(mud->mqtt_state.pending_msg_q), temp_msg, 
                      msg_id, MQTT_MSG_TYPE_PUBREL, (int)mqtt_get_qos(temp_msg->data) );
            NODE_DBG("MQTT: Response PUBREL\r\n");
          }
          break;
        case MQTT_MSG_TYPE_PUBREL:
          if(pending_msg && pending_msg->msg_type == MQTT_MSG_TYPE_PUBREC && pending_msg->msg_id == msg_id){
            msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
            temp_msg = mqtt_msg_pubcomp(&mud->mqtt_state.mqtt_connection, msg_id);
            node = msg_enqueue(&(mud->mqtt_state.pending_msg_q), temp_msg, 
                      msg_id, MQTT_MSG_TYPE_PUBCOMP, (int)mqtt_get_qos(temp_msg->data) );
            NODE_DBG("MQTT: Response PUBCOMP\r\n");
          }
          break;
        case MQTT_MSG_TYPE_PUBCOMP:
          if(pending_msg && pending_msg->msg_type == MQTT_MSG_TYPE_PUBREL && pending_msg->msg_id == msg_id){
            NODE_DBG("MQTT: Publish  with QoS = 2 successful\r\n");
            msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
            if(mud->cb_puback_ref == LUA_NOREF)
              break;
            if(mud->self_ref == LUA_NOREF)
              break;
            if(mud->L == NULL)
              break;
            lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->cb_puback_ref);
            lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->self_ref);  // pass the userdata to callback func in lua
            lua_call(mud->L, 1, 0);
          }
          break;
        case MQTT_MSG_TYPE_PINGREQ:
            temp_msg = mqtt_msg_pingresp(&mud->mqtt_state.mqtt_connection);
            node = msg_enqueue(&(mud->mqtt_state.pending_msg_q), temp_msg, 
                      msg_id, MQTT_MSG_TYPE_PINGRESP, (int)mqtt_get_qos(temp_msg->data) );
            NODE_DBG("MQTT: Response PINGRESP\r\n");
          break;
        case MQTT_MSG_TYPE_PINGRESP:
          // Ignore
          NODE_DBG("MQTT: PINGRESP received\r\n");
          break;
      }
      // NOTE: this is done down here and not in the switch case above
      // because the PSOCK_READBUF_LEN() won't work inside a switch
      // statement due to the way protothreads resume.
      if(msg_type == MQTT_MSG_TYPE_PUBLISH)
      {

        length = mud->mqtt_state.message_length_read;

        if(mud->mqtt_state.message_length < mud->mqtt_state.message_length_read)
				{
					length -= mud->mqtt_state.message_length;
					in_buffer += mud->mqtt_state.message_length;

					NODE_DBG("Get another published message\r\n");
					goto READPACKET;
				}
      }
      break;
  }

  if(node && (1==msg_size(&(mud->mqtt_state.pending_msg_q))) && mud->event_timeout == 0){
    mud->event_timeout = MQTT_SEND_TIMEOUT;
    NODE_DBG("Sent: %d\n", node->msg.length);
    if( mud->secure )
      espconn_secure_sent( pesp_conn, node->msg.data, node->msg.length );
    else
      espconn_sent( pesp_conn, node->msg.data, node->msg.length );
  }
  mud->keep_alive_tick = 0;
  NODE_DBG("receive, queue size: %d\n", msg_size(&(mud->mqtt_state.pending_msg_q)));
  NODE_DBG("leave mqtt_socket_received.\n");
  return;
}

static void mqtt_socket_sent(void *arg)
{
  NODE_DBG("enter mqtt_socket_sent.\n");
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL)
    return;
  lmqtt_userdata *mud = (lmqtt_userdata *)pesp_conn->reverse;
  if(mud == NULL)
    return;
  if(!mud->connected)
    return;
  // call mqtt_sent()
  mud->event_timeout = 0;
  mud->keep_alive_tick = 0;

  if(mud->connState == MQTT_CONNECT_SENDING){
    mud->connState = MQTT_CONNECT_SENT;
    // MQTT_CONNECT not queued.
    return;
  }
  NODE_DBG("sent1, queue size: %d\n", msg_size(&(mud->mqtt_state.pending_msg_q)));
  // qos = 0, publish and forgot.
  msg_queue_t *node = msg_peek(&(mud->mqtt_state.pending_msg_q));
  if(node && node->msg_type == MQTT_MSG_TYPE_PUBLISH && node->publish_qos == 0) {
    msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
  	if(mud->cb_puback_ref == LUA_NOREF)
			return;
		if(mud->self_ref == LUA_NOREF)
			return;
    if(mud->L == NULL)
      return;
		lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->cb_puback_ref);
		lua_rawgeti(mud->L, LUA_REGISTRYINDEX, mud->self_ref);  // pass the userdata to callback func in lua
		lua_call(mud->L, 1, 0);
  } else if(node && node->msg_type == MQTT_MSG_TYPE_PUBACK && node->publish_qos == 1) {
    msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
  } else if(node && node->msg_type == MQTT_MSG_TYPE_PUBCOMP) {
    msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
  } else if(node && node->msg_type == MQTT_MSG_TYPE_PINGREQ) {
    msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
  }
  NODE_DBG("sent2, queue size: %d\n", msg_size(&(mud->mqtt_state.pending_msg_q)));
  NODE_DBG("leave mqtt_socket_sent.\n");
}

static void mqtt_socket_connected(void *arg)
{
  NODE_DBG("enter mqtt_socket_connected.\n");
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL)
    return;
  lmqtt_userdata *mud = (lmqtt_userdata *)pesp_conn->reverse;
  if(mud == NULL)
    return;
  mud->connected = true;
  espconn_regist_recvcb(pesp_conn, mqtt_socket_received);
  espconn_regist_sentcb(pesp_conn, mqtt_socket_sent);
  espconn_regist_disconcb(pesp_conn, mqtt_socket_disconnected);

  uint8_t temp_buffer[MQTT_BUF_SIZE];
  // call mqtt_connect() to start a mqtt connect stage.
  mqtt_msg_init(&mud->mqtt_state.mqtt_connection, temp_buffer, MQTT_BUF_SIZE);
  mqtt_message_t* temp_msg = mqtt_msg_connect(&mud->mqtt_state.mqtt_connection, mud->mqtt_state.connect_info);
  NODE_DBG("Send MQTT connection infomation, data len: %d, d[0]=%d \r\n", temp_msg->length,  temp_msg->data[0]);
  mud->event_timeout = MQTT_SEND_TIMEOUT;
  // not queue this message. should send right now. or should enqueue this before head.
  if(mud->secure)
    espconn_secure_sent(pesp_conn, temp_msg->data, temp_msg->length);
  else
    espconn_sent(pesp_conn, temp_msg->data, temp_msg->length);
  mud->keep_alive_tick = 0;

  mud->connState = MQTT_CONNECT_SENDING;
  NODE_DBG("leave mqtt_socket_connected.\n");
  return;
}

void mqtt_socket_timer(void *arg)
{
  NODE_DBG("enter mqtt_socket_timer.\n");
  lmqtt_userdata *mud = (lmqtt_userdata*) arg;

  if(mud == NULL)
    return;
  if(mud->pesp_conn == NULL){
    NODE_DBG("mud->pesp_conn is NULL.\n");
    os_timer_disarm(&mud->mqttTimer);
    return;
  }

  NODE_DBG("timer, queue size: %d\n", msg_size(&(mud->mqtt_state.pending_msg_q)));
  if(mud->event_timeout > 0){
    NODE_DBG("event_timeout: %d.\n", mud->event_timeout);
  	mud->event_timeout --;
    if(mud->event_timeout > 0){
      return;
    } else {
      NODE_DBG("event timeout. \n");
      if(mud->connState == MQTT_DATA)
        msg_destroy(msg_dequeue(&(mud->mqtt_state.pending_msg_q)));
      // should remove the head of the queue and re-send with DUP = 1
      // Not implemented yet.
    }
  }

  if(mud->connState == MQTT_INIT){ // socket connect time out.
    NODE_DBG("Can not connect to broker.\n");
    // Never goes here.
  } else if(mud->connState == MQTT_CONNECT_SENDING){ // MQTT_CONNECT send time out.
    NODE_DBG("sSend MQTT_CONNECT failed.\n");
    mud->connState = MQTT_INIT;
    if(mud->secure)
      espconn_secure_disconnect(mud->pesp_conn);
    else 
      espconn_disconnect(mud->pesp_conn);
    mud->keep_alive_tick = 0; // not need count anymore
  } else if(mud->connState == MQTT_CONNECT_SENT){ // wait for CONACK time out.
    NODE_DBG("MQTT_CONNECT failed.\n");
  } else if(mud->connState == MQTT_DATA){
    msg_queue_t *pending_msg = msg_peek(&(mud->mqtt_state.pending_msg_q));
    if(pending_msg){
      mud->event_timeout = MQTT_SEND_TIMEOUT;
      if(mud->secure)
        espconn_secure_sent(mud->pesp_conn, pending_msg->msg.data, pending_msg->msg.length);
      else
        espconn_sent(mud->pesp_conn, pending_msg->msg.data, pending_msg->msg.length);
      mud->keep_alive_tick = 0;
      NODE_DBG("id: %d - qos: %d, length: %d\n", pending_msg->msg_id, pending_msg->publish_qos, pending_msg->msg.length);
    } else {
      // no queued event.
      mud->keep_alive_tick ++;
      if(mud->keep_alive_tick > mud->mqtt_state.connect_info->keepalive){
        mud->event_timeout = MQTT_SEND_TIMEOUT;
        uint8_t temp_buffer[MQTT_BUF_SIZE];
        mqtt_msg_init(&mud->mqtt_state.mqtt_connection, temp_buffer, MQTT_BUF_SIZE);
        NODE_DBG("\r\nMQTT: Send keepalive packet\r\n");
        mqtt_message_t* temp_msg = mqtt_msg_pingreq(&mud->mqtt_state.mqtt_connection);
        msg_queue_t *node = msg_enqueue( &(mud->mqtt_state.pending_msg_q), temp_msg, 
                            0, MQTT_MSG_TYPE_PINGREQ, (int)mqtt_get_qos(temp_msg->data) );
        // only one message in queue, send immediately.
        if(mud->secure)
          espconn_secure_sent(mud->pesp_conn, temp_msg->data, temp_msg->length);
        else
          espconn_sent(mud->pesp_conn, temp_msg->data, temp_msg->length);
        mud->keep_alive_tick = 0;
      }
    }
  }
  NODE_DBG("leave mqtt_socket_timer.\n");
}

// Lua: mqtt.Client(clientid, keepalive, user, pass)
static int mqtt_socket_client( lua_State* L )
{
  NODE_DBG("enter mqtt_socket_client.\n");

  lmqtt_userdata *mud;
  char tempid[20] = {0};
  c_sprintf(tempid, "%s%x", "NodeMCU_", system_get_chip_id() );
  NODE_DBG(tempid);
  NODE_DBG("\n");
  
  const char *clientId = tempid, *username = NULL, *password = NULL;
  size_t idl = c_strlen(tempid);
  size_t unl = 0, pwl = 0;
  int keepalive = 0;
  int stack = 1;
  unsigned secure = 0;
  int top = lua_gettop(L);

  // create a object
  mud = (lmqtt_userdata *)lua_newuserdata(L, sizeof(lmqtt_userdata));
  // pre-initialize it, in case of errors
  mud->L = NULL;
  mud->self_ref = LUA_NOREF;
  mud->cb_connect_ref = LUA_NOREF;
  mud->cb_disconnect_ref = LUA_NOREF;

  mud->cb_message_ref = LUA_NOREF;
  mud->cb_suback_ref = LUA_NOREF;
  mud->cb_puback_ref = LUA_NOREF;
  mud->pesp_conn = NULL;
  mud->secure = 0;

  mud->keep_alive_tick = 0;
  mud->event_timeout = 0;
  mud->connState = MQTT_INIT;
  mud->connected = false;
  c_memset(&mud->mqttTimer, 0, sizeof(ETSTimer));
  c_memset(&mud->mqtt_state, 0, sizeof(mqtt_state_t));
  c_memset(&mud->connect_info, 0, sizeof(mqtt_connect_info_t));

  // set its metatable
  luaL_getmetatable(L, "mqtt.socket");
  lua_setmetatable(L, -2);

  mud->L = L;   // L for mqtt module.

  if( lua_isstring(L,stack) )   // deal with the clientid string
  {
    clientId = luaL_checklstring( L, stack, &idl );
    stack++;
  }

  if(lua_isnumber( L, stack ))
  {   
    keepalive = luaL_checkinteger( L, stack);
    stack++;
  }

  if(keepalive == 0){
    keepalive = MQTT_DEFAULT_KEEPALIVE;
  }

  if(lua_isstring( L, stack )){
    username = luaL_checklstring( L, stack, &unl );
    stack++;
  }
  if(username == NULL)
    unl = 0;
  NODE_DBG("lengh username: %d\r\n", unl);

  if(lua_isstring( L, stack )){
    password = luaL_checklstring( L, stack, &pwl );
    stack++;
  }
  if(password == NULL)
    pwl = 0;
  NODE_DBG("lengh password: %d\r\n", pwl);

  // TODO: check the zalloc result.
  mud->connect_info.client_id = (uint8_t *)c_zalloc(idl+1);
  mud->connect_info.username = (uint8_t *)c_zalloc(unl + 1);
  mud->connect_info.password = (uint8_t *)c_zalloc(pwl + 1);
  if(!mud->connect_info.client_id || !mud->connect_info.username || !mud->connect_info.password){
    if(mud->connect_info.client_id) {
      c_free(mud->connect_info.client_id);
      mud->connect_info.client_id = NULL;
    }
    if(mud->connect_info.username) {
      c_free(mud->connect_info.username);
      mud->connect_info.username = NULL;
    }
    if(mud->connect_info.password) {
      c_free(mud->connect_info.password);
      mud->connect_info.password = NULL;
    }
  	return luaL_error(L, "not enough memory");
  }

  c_memcpy(mud->connect_info.client_id, clientId, idl);
  mud->connect_info.client_id[idl] = 0;
  c_memcpy(mud->connect_info.username, username, unl);
  mud->connect_info.username[unl] = 0;
  c_memcpy(mud->connect_info.password, password, pwl);
  mud->connect_info.password[pwl] = 0;
  
  NODE_DBG("MQTT: Init info: %s, %s, %s\r\n", mud->connect_info.client_id, mud->connect_info.username, mud->connect_info.password);

  mud->connect_info.clean_session = 1;
  mud->connect_info.will_qos = 0;
  mud->connect_info.will_retain = 0;
  mud->connect_info.keepalive = keepalive;

  mud->mqtt_state.pending_msg_q = NULL;
  mud->mqtt_state.auto_reconnect = 0;
  mud->mqtt_state.port = 1883;
  mud->mqtt_state.connect_info = &mud->connect_info;

  NODE_DBG("leave mqtt_socket_client.\n");
  return 1;
}

// Lua: mqtt.delete( socket )
// call close() first
// socket: unref everything
static int mqtt_delete( lua_State* L )
{
  NODE_DBG("enter mqtt_delete.\n");

  lmqtt_userdata *mud = (lmqtt_userdata *)luaL_checkudata(L, 1, "mqtt.socket");
  luaL_argcheck(L, mud, 1, "mqtt.socket expected");
  if(mud==NULL){
    NODE_DBG("userdata is nil.\n");
    return 0;
  }

  os_timer_disarm(&mud->mqttTimer);
  mud->connected = false;

  // ---- alloc-ed in mqtt_socket_connect()
  if(mud->pesp_conn){     // for client connected to tcp server, this should set NULL in disconnect cb
    mud->pesp_conn->reverse = NULL;
    if(mud->pesp_conn->proto.tcp)
      c_free(mud->pesp_conn->proto.tcp);
    mud->pesp_conn->proto.tcp = NULL;
    c_free(mud->pesp_conn);
    mud->pesp_conn = NULL;    // for socket, it will free this when disconnected
  }

  // ---- alloc-ed in mqtt_socket_lwt()
  if(mud->connect_info.will_topic){
  	c_free(mud->connect_info.will_topic);
  	mud->connect_info.will_topic = NULL;
  }

  if(mud->connect_info.will_message){
    c_free(mud->connect_info.will_message);
    mud->connect_info.will_message = NULL;
  }
  // ----

  //--------- alloc-ed in mqtt_socket_client()
  if(mud->connect_info.client_id){
    c_free(mud->connect_info.client_id);
    mud->connect_info.client_id = NULL;
  }
  if(mud->connect_info.username){
    c_free(mud->connect_info.username);
    mud->connect_info.username = NULL;
  }
  if(mud->connect_info.password){
    c_free(mud->connect_info.password);
    mud->connect_info.password = NULL;
  }
  // -------

  // free (unref) callback ref
  if(LUA_NOREF!=mud->cb_connect_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_connect_ref);
    mud->cb_connect_ref = LUA_NOREF;
  }
  if(LUA_NOREF!=mud->cb_disconnect_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_disconnect_ref);
    mud->cb_disconnect_ref = LUA_NOREF;
  }
  if(LUA_NOREF!=mud->cb_message_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_message_ref);
    mud->cb_message_ref = LUA_NOREF;
  }
  if(LUA_NOREF!=mud->cb_suback_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_suback_ref);
    mud->cb_suback_ref = LUA_NOREF;
  }
  if(LUA_NOREF!=mud->cb_puback_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_puback_ref);
    mud->cb_puback_ref = LUA_NOREF;
  }
  lua_gc(L, LUA_GCSTOP, 0);
  if(LUA_NOREF!=mud->self_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, mud->self_ref);
    mud->self_ref = LUA_NOREF;
  }
  lua_gc(L, LUA_GCRESTART, 0);
  NODE_DBG("leave mqtt_delete.\n");
  return 0;  
}

static void socket_connect(struct espconn *pesp_conn)
{
  NODE_DBG("enter socket_connect.\n");
  if(pesp_conn == NULL)
    return;
  lmqtt_userdata *mud = (lmqtt_userdata *)pesp_conn->reverse;
  if(mud == NULL)
    return;

  mud->event_timeout = MQTT_CONNECT_TIMEOUT;
  mud->connState = MQTT_INIT;
  if(mud->secure)
    espconn_secure_connect(pesp_conn);
  else
    espconn_connect(pesp_conn);

  os_timer_arm(&mud->mqttTimer, 1000, 1);
  
  NODE_DBG("leave socket_connect.\n");
}

static void socket_dns_found(const char *name, ip_addr_t *ipaddr, void *arg);
static dns_reconn_count = 0;
static ip_addr_t host_ip; // for dns
static void socket_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
  NODE_DBG("enter socket_dns_found.\n");
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL){
    NODE_DBG("pesp_conn null.\n");
    return;
  }

  if(ipaddr == NULL)
  {
    dns_reconn_count++;
    if( dns_reconn_count >= 5 ){
      NODE_ERR( "DNS Fail!\n" );
      // Note: should delete the pesp_conn or unref self_ref here.
      mqtt_socket_disconnected(arg);   // although not connected, but fire disconnect callback to release every thing.
      return;
    }
    NODE_ERR( "DNS retry %d!\n", dns_reconn_count );
    host_ip.addr = 0;
    espconn_gethostbyname(pesp_conn, name, &host_ip, socket_dns_found);
    return;
  }

  // ipaddr->addr is a uint32_t ip
  if(ipaddr->addr != 0)
  {
    dns_reconn_count = 0;
    c_memcpy(pesp_conn->proto.tcp->remote_ip, &(ipaddr->addr), 4);
    NODE_DBG("TCP ip is set: ");
    NODE_DBG(IPSTR, IP2STR(&(ipaddr->addr)));
    NODE_DBG("\n");
    socket_connect(pesp_conn);
  }
  NODE_DBG("leave socket_dns_found.\n");
}

// Lua: mqtt:connect( host, port, secure, auto_reconnect, function(client) )
static int mqtt_socket_connect( lua_State* L )
{
  NODE_DBG("enter mqtt_socket_connect.\n");
  lmqtt_userdata *mud = NULL;
  unsigned port = 1883;
  size_t il;
  ip_addr_t ipaddr;
  const char *domain;
  int stack = 1;
  unsigned secure = 0, auto_reconnect = 0;
  int top = lua_gettop(L);

  mud = (lmqtt_userdata *)luaL_checkudata(L, stack, "mqtt.socket");
  luaL_argcheck(L, mud, stack, "mqtt.socket expected");
  stack++;
  if(mud == NULL)
    return 0;

  if(mud->connected){
    return luaL_error(L, "already connected");
  }

  if(mud->pesp_conn){   //TODO: should I free tcp struct directly or ask user to call close()???
    mud->pesp_conn->reverse = NULL;
    if(mud->pesp_conn->proto.tcp)
      c_free(mud->pesp_conn->proto.tcp);
    mud->pesp_conn->proto.tcp = NULL;
    c_free(mud->pesp_conn);
    mud->pesp_conn = NULL;
  }

  struct espconn *pesp_conn = NULL;
	pesp_conn = mud->pesp_conn = (struct espconn *)c_zalloc(sizeof(struct espconn));
	if(!pesp_conn)
		return luaL_error(L, "not enough memory");

	pesp_conn->proto.udp = NULL;
	pesp_conn->proto.tcp = (esp_tcp *)c_zalloc(sizeof(esp_tcp));
	if(!pesp_conn->proto.tcp){
		c_free(pesp_conn);
		pesp_conn = mud->pesp_conn = NULL;
		return luaL_error(L, "not enough memory");
	}
	// reverse is for the callback function
	pesp_conn->reverse = mud;
	pesp_conn->type = ESPCONN_TCP;
	pesp_conn->state = ESPCONN_NONE;
  mud->connected = false;

  if( (stack<=top) && lua_isstring(L,stack) )   // deal with the domain string
  {
    domain = luaL_checklstring( L, stack, &il );

    stack++;
    if (domain == NULL)
    {
      domain = "127.0.0.1";
    }
    ipaddr.addr = ipaddr_addr(domain);
    c_memcpy(pesp_conn->proto.tcp->remote_ip, &ipaddr.addr, 4);
    NODE_DBG("TCP ip is set: ");
    NODE_DBG(IPSTR, IP2STR(&ipaddr.addr));
    NODE_DBG("\n");
  }

  if ( (stack<=top) && lua_isnumber(L, stack) )
  {
    port = lua_tointeger(L, stack);
    stack++;
    NODE_DBG("TCP port is set: %d.\n", port);
  }
  pesp_conn->proto.tcp->remote_port = port;
  pesp_conn->proto.tcp->local_port = espconn_port();
  mud->mqtt_state.port = port;

  if ( (stack<=top) && lua_isnumber(L, stack) )
  {
    secure = lua_tointeger(L, stack);
    stack++;
    if ( secure != 0 && secure != 1 ){
      secure = 0; // default to 0
    }
  } else {
    secure = 0; // default to 0
  }
  mud->secure = secure; // save

  if ( (stack<=top) && lua_isnumber(L, stack) )
  {
    auto_reconnect = lua_tointeger(L, stack);
    stack++;
    if ( auto_reconnect != 0 && auto_reconnect != 1 ){
      auto_reconnect = 0; // default to 0
    }
  } else {
    auto_reconnect = 0; // default to 0
  }
  mud->mqtt_state.auto_reconnect = auto_reconnect;

  // call back function when a connection is obtained, tcp only
  if ((stack<=top) && (lua_type(L, stack) == LUA_TFUNCTION || lua_type(L, stack) == LUA_TLIGHTFUNCTION)){
    lua_pushvalue(L, stack);  // copy argument (func) to the top of stack
    if(mud->cb_connect_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_connect_ref);
    mud->cb_connect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    stack++;
  }

  lua_pushvalue(L, 1);  // copy userdata to the top of stack
  if(mud->self_ref != LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, mud->self_ref);
  mud->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

  espconn_regist_connectcb(pesp_conn, mqtt_socket_connected);
  espconn_regist_reconcb(pesp_conn, mqtt_socket_reconnected);

  os_timer_disarm(&mud->mqttTimer);
  os_timer_setfn(&mud->mqttTimer, (os_timer_func_t *)mqtt_socket_timer, mud);
  // timer started in socket_connect()

  if((ipaddr.addr == IPADDR_NONE) && (c_memcmp(domain,"255.255.255.255",16) != 0))
  {
    host_ip.addr = 0;
    dns_reconn_count = 0;
    if(ESPCONN_OK == espconn_gethostbyname(pesp_conn, domain, &host_ip, socket_dns_found)){
      socket_dns_found(domain, &host_ip, pesp_conn);  // ip is returned in host_ip.
    }
  }
  else
  {
    socket_connect(pesp_conn);
  }

  NODE_DBG("leave mqtt_socket_connect.\n");
  return 0;
}

// Lua: mqtt:close()
// client disconnect and unref itself
static int mqtt_socket_close( lua_State* L )
{
  NODE_DBG("enter mqtt_socket_close.\n");
  int i = 0;
  lmqtt_userdata *mud = NULL;

  mud = (lmqtt_userdata *)luaL_checkudata(L, 1, "mqtt.socket");
  luaL_argcheck(L, mud, 1, "mqtt.socket expected");
  if(mud == NULL)
    return 0;

  if(mud->pesp_conn == NULL)
    return 0;

  // call mqtt_disconnect()
  mud->mqtt_state.auto_reconnect = 0;   // stop auto reconnect.

  if(mud->secure){
    if(mud->pesp_conn->proto.tcp->remote_port || mud->pesp_conn->proto.tcp->local_port)
      espconn_secure_disconnect(mud->pesp_conn);
  }
  else
  {
    if(mud->pesp_conn->proto.tcp->remote_port || mud->pesp_conn->proto.tcp->local_port)
      espconn_disconnect(mud->pesp_conn);
  }
  NODE_DBG("leave mqtt_socket_close.\n");
  return 0;  
}

// Lua: mqtt:on( "method", function() )
static int mqtt_socket_on( lua_State* L )
{
  NODE_DBG("enter mqtt_socket_on.\n");
  lmqtt_userdata *mud;
  size_t sl;

  mud = (lmqtt_userdata *)luaL_checkudata(L, 1, "mqtt.socket");
  luaL_argcheck(L, mud, 1, "mqtt.socket expected");
  if(mud==NULL){
    NODE_DBG("userdata is nil.\n");
    return 0;
  }

  const char *method = luaL_checklstring( L, 2, &sl );
  if (method == NULL)
    return luaL_error( L, "wrong arg type" );

  luaL_checkanyfunction(L, 3);
  lua_pushvalue(L, 3);  // copy argument (func) to the top of stack

  if( sl == 7 && c_strcmp(method, "connect") == 0){
    if(mud->cb_connect_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_connect_ref);
    mud->cb_connect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }else if( sl == 7 && c_strcmp(method, "offline") == 0){
    if(mud->cb_disconnect_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_disconnect_ref);
    mud->cb_disconnect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }else if( sl == 7 && c_strcmp(method, "message") == 0){
    if(mud->cb_message_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_message_ref);
    mud->cb_message_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }else{
    lua_pop(L, 1);
    return luaL_error( L, "method not supported" );
  }
  NODE_DBG("leave mqtt_socket_on.\n");
  return 0;
}

// Lua: bool = mqtt:subscribe(topic, qos, function())
static int mqtt_socket_subscribe( lua_State* L ) {
	NODE_DBG("enter mqtt_socket_subscribe.\n");

	uint8_t stack = 1, qos = 0;
  uint16_t msg_id = 0;
	const char *topic;
	size_t il;
	lmqtt_userdata *mud;

	mud = (lmqtt_userdata *) luaL_checkudata( L, stack, "mqtt.socket" );
	luaL_argcheck( L, mud, stack, "mqtt.socket expected" );
	stack++;

  if(mud==NULL){
    NODE_DBG("userdata is nil.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

  if(mud->pesp_conn == NULL){
    NODE_DBG("mud->pesp_conn is NULL.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

	if(!mud->connected){
		luaL_error( L, "not connected" );
    lua_pushboolean(L, 0);
    return 1;
  }

  uint8_t temp_buffer[MQTT_BUF_SIZE];
  mqtt_msg_init(&mud->mqtt_state.mqtt_connection, temp_buffer, MQTT_BUF_SIZE);
  mqtt_message_t *temp_msg = NULL;

	if( lua_istable( L, stack ) ) {
		NODE_DBG("subscribe table\n");
		lua_pushnil( L ); /* first key */

    uint8_t temp_buf[MQTT_BUF_SIZE];
    uint32_t temp_pos = 0;

		while( lua_next( L, stack ) != 0 ) {
			topic = luaL_checkstring( L, -2 );
			qos = luaL_checkinteger( L, -1 );

			temp_msg = mqtt_msg_subscribe( &mud->mqtt_state.mqtt_connection, topic, qos, &msg_id );
			NODE_DBG("topic: %s - qos: %d, length: %d\n", topic, qos, temp_msg->length);

      if (temp_pos + temp_msg->length > MQTT_BUF_SIZE){
        lua_pop(L, 1);
        break;  // too long message for the outbuffer.
      }
      c_memcpy( temp_buf + temp_pos, temp_msg->data, temp_msg->length );
      temp_pos += temp_msg->length;

			lua_pop( L, 1 );
		}

    if (temp_pos == 0){
      luaL_error( L, "invalid data" );
      lua_pushboolean(L, 0);
      return 1;
    }

    c_memcpy( temp_buffer, temp_buf, temp_pos );
		temp_msg->data = temp_buffer;
		temp_msg->length = temp_pos;
		stack++;
	} else {
		NODE_DBG("subscribe string\n");
		topic = luaL_checklstring( L, stack, &il );
		stack++;
		if( topic == NULL ){
			luaL_error( L, "need topic name" );
      lua_pushboolean(L, 0);
      return 1;
    }
		qos = luaL_checkinteger( L, stack );
		temp_msg = mqtt_msg_subscribe( &mud->mqtt_state.mqtt_connection, topic, qos, &msg_id );
		stack++;
	}

  if( lua_type( L, stack ) == LUA_TFUNCTION || lua_type( L, stack ) == LUA_TLIGHTFUNCTION ) {    // TODO: this will overwrite the previous one.
    lua_pushvalue( L, stack );  // copy argument (func) to the top of stack
    if( mud->cb_suback_ref != LUA_NOREF )
      luaL_unref( L, LUA_REGISTRYINDEX, mud->cb_suback_ref );
    mud->cb_suback_ref = luaL_ref( L, LUA_REGISTRYINDEX );
  }

  msg_queue_t *node = msg_enqueue( &(mud->mqtt_state.pending_msg_q), temp_msg, 
                            msg_id, MQTT_MSG_TYPE_SUBSCRIBE, (int)mqtt_get_qos(temp_msg->data) );

  NODE_DBG("topic: %s - id: %d - qos: %d, length: %d\n", topic, node->msg_id, node->publish_qos, node->msg.length);

  if(node && (1==msg_size(&(mud->mqtt_state.pending_msg_q))) && mud->event_timeout == 0){
  	mud->event_timeout = MQTT_SEND_TIMEOUT;
  	NODE_DBG("Sent: %d\n", node->msg.length);
  	if( mud->secure )
  		espconn_secure_sent( mud->pesp_conn, node->msg.data, node->msg.length );
  	else
  		espconn_sent( mud->pesp_conn, node->msg.data, node->msg.length );
    mud->keep_alive_tick = 0;
  }

  if(!node){
    lua_pushboolean(L, 0);
  } else {
    lua_pushboolean(L, 1);  // enqueued succeed.
  }
  NODE_DBG("subscribe, queue size: %d\n", msg_size(&(mud->mqtt_state.pending_msg_q)));
  NODE_DBG("leave mqtt_socket_subscribe.\n");
	return 1;
}

// Lua: bool = mqtt:publish( topic, payload, qos, retain, function() )
static int mqtt_socket_publish( lua_State* L )
{
  NODE_DBG("enter mqtt_socket_publish.\n");
  struct espconn *pesp_conn = NULL;
  lmqtt_userdata *mud;
  size_t l;
  uint8_t stack = 1;
  uint16_t msg_id = 0;
  mud = (lmqtt_userdata *)luaL_checkudata(L, stack, "mqtt.socket");
  luaL_argcheck(L, mud, stack, "mqtt.socket expected");
  stack++;
  if(mud==NULL){
    NODE_DBG("userdata is nil.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

  if(mud->pesp_conn == NULL){
    NODE_DBG("mud->pesp_conn is NULL.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

  if(!mud->connected){
    luaL_error( L, "not connected" );
    lua_pushboolean(L, 0);
    return 1;
  }

  const char *topic = luaL_checklstring( L, stack, &l );
  stack ++;
  if (topic == NULL){
    luaL_error( L, "need topic" );
    lua_pushboolean(L, 0);
    return 1;
  }

  const char *payload = luaL_checklstring( L, stack, &l );
  stack ++;
  uint8_t qos = luaL_checkinteger( L, stack);
  stack ++;
  uint8_t retain = luaL_checkinteger( L, stack);
  stack ++;

  uint8_t temp_buffer[MQTT_BUF_SIZE];
  mqtt_msg_init(&mud->mqtt_state.mqtt_connection, temp_buffer, MQTT_BUF_SIZE);
  mqtt_message_t *temp_msg = mqtt_msg_publish(&mud->mqtt_state.mqtt_connection,
                       topic, payload, l,
                       qos, retain,
                       &msg_id);

  if (lua_type(L, stack) == LUA_TFUNCTION || lua_type(L, stack) == LUA_TLIGHTFUNCTION){
    lua_pushvalue(L, stack);  // copy argument (func) to the top of stack
    if(mud->cb_puback_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, mud->cb_puback_ref);
    mud->cb_puback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  msg_queue_t *node = msg_enqueue(&(mud->mqtt_state.pending_msg_q), temp_msg, 
                      msg_id, MQTT_MSG_TYPE_PUBLISH, (int)qos );

  if(node && (1==msg_size(&(mud->mqtt_state.pending_msg_q))) && mud->event_timeout == 0){
    mud->event_timeout = MQTT_SEND_TIMEOUT;
    NODE_DBG("Sent: %d\n", node->msg.length);
    if( mud->secure )
      espconn_secure_sent( mud->pesp_conn, node->msg.data, node->msg.length );
    else
      espconn_sent( mud->pesp_conn, node->msg.data, node->msg.length );
    mud->keep_alive_tick = 0;
  }

  if(!node){
    lua_pushboolean(L, 0);
  } else {
    lua_pushboolean(L, 1);  // enqueued succeed.
  }

  NODE_DBG("publish, queue size: %d\n", msg_size(&(mud->mqtt_state.pending_msg_q)));
  NODE_DBG("leave mqtt_socket_publish.\n");
  return 1;
}

// Lua: mqtt:lwt( topic, message, qos, retain, function(client) )
static int mqtt_socket_lwt( lua_State* L )
{
  NODE_DBG("enter mqtt_socket_lwt.\n");
  uint8_t stack = 1;
  size_t topicSize, msgSize;
  NODE_DBG("mqtt_socket_lwt.\n");
  lmqtt_userdata *mud = NULL;
  const char *lwtTopic, *lwtMsg;
  uint8_t lwtQoS, lwtRetain;

  mud = (lmqtt_userdata *)luaL_checkudata( L, stack, "mqtt.socket" );
  luaL_argcheck( L, mud, stack, "mqtt.socket expected" );

  if(mud == NULL)
    return 0;

  stack++;
  lwtTopic = luaL_checklstring( L, stack, &topicSize );
  if (lwtTopic == NULL)
  {
    return luaL_error( L, "need lwt topic");
  }

  stack++;
  lwtMsg = luaL_checklstring( L, stack, &msgSize );
  if (lwtMsg == NULL)
  {
    return luaL_error( L, "need lwt message");
  }

  if(mud->connect_info.will_topic){    // free the previous one if there is any
    c_free(mud->connect_info.will_topic);
    mud->connect_info.will_topic = NULL;
  }
  if(mud->connect_info.will_message){
    c_free(mud->connect_info.will_message);
    mud->connect_info.will_message = NULL;
  }

  mud->connect_info.will_topic = (uint8_t*) c_zalloc( topicSize + 1 );
  mud->connect_info.will_message = (uint8_t*) c_zalloc( msgSize + 1 );
  if(!mud->connect_info.will_topic || !mud->connect_info.will_message){
    if(mud->connect_info.will_topic){
      c_free(mud->connect_info.will_topic);
      mud->connect_info.will_topic = NULL;
    }
    if(mud->connect_info.will_message){
      c_free(mud->connect_info.will_message);
      mud->connect_info.will_message = NULL;
    }
    return luaL_error( L, "not enough memory");
  }
  c_memcpy(mud->connect_info.will_topic, lwtTopic, topicSize);
  mud->connect_info.will_topic[topicSize] = 0;
  c_memcpy(mud->connect_info.will_message, lwtMsg, msgSize);
  mud->connect_info.will_message[msgSize] = 0;

  if ( lua_isnumber(L, stack) )
  {
    mud->connect_info.will_qos = lua_tointeger(L, stack);
    stack++;
  }
  if ( lua_isnumber(L, stack) )
  {
    mud->connect_info.will_retain = lua_tointeger(L, stack);
    stack++;
  }

  NODE_DBG("mqtt_socket_lwt: topic: %s, message: %s, qos: %d, retain: %d\n",
      mud->connect_info.will_topic,
      mud->connect_info.will_message,
      mud->connect_info.will_qos,
      mud->connect_info.will_retain);
  NODE_DBG("leave mqtt_socket_lwt.\n");
  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

static const LUA_REG_TYPE mqtt_socket_map[] =
{
  { LSTRKEY( "connect" ), LFUNCVAL ( mqtt_socket_connect ) },
  { LSTRKEY( "close" ), LFUNCVAL ( mqtt_socket_close ) },
  { LSTRKEY( "publish" ), LFUNCVAL ( mqtt_socket_publish ) },
  { LSTRKEY( "subscribe" ), LFUNCVAL ( mqtt_socket_subscribe ) },
  { LSTRKEY( "lwt" ), LFUNCVAL ( mqtt_socket_lwt ) },
  { LSTRKEY( "on" ), LFUNCVAL ( mqtt_socket_on ) },
  { LSTRKEY( "__gc" ), LFUNCVAL ( mqtt_delete ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__index" ), LROVAL ( mqtt_socket_map ) },
#endif
  { LNILKEY, LNILVAL }
};

const LUA_REG_TYPE mqtt_map[] = 
{
  { LSTRKEY( "Client" ), LFUNCVAL ( mqtt_socket_client ) },
#if LUA_OPTIMIZE_MEMORY > 0

  { LSTRKEY( "__metatable" ), LROVAL( mqtt_map ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_mqtt( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  luaL_rometatable(L, "mqtt.socket", (void *)mqtt_socket_map);  // create metatable for mqtt.socket
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  int n;
  luaL_register( L, AUXLIB_MQTT, mqtt_map );

  // Set it as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );

  // Module constants  
  // MOD_REG_NUMBER( L, "TCP", TCP );

  // create metatable
  luaL_newmetatable(L, "mqtt.socket");
  // metatable.__index = metatable
  lua_pushliteral(L, "__index");
  lua_pushvalue(L,-2);
  lua_rawset(L,-3);
  // Setup the methods inside metatable
  luaL_register( L, NULL, mqtt_socket_map );

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
