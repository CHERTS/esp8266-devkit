var now = new Date();
var timenow = now.getHours() + (now.getMinutes() / 60);
var days = {
    0: 'sun',
    1: 'mon',
    2: 'tue',
    3: 'wed',
    4: 'thu',
    5: 'fri',
    6: 'sat',
    7: 'sun'
};
var today = days[now.getDay()];

//=================================================
// DATA
//=================================================

var visibleFlag = 1;

var statusMsg = false;
var connected = false;

var thermostat = {
	roomtemperature: "21",
	relay1state: 0,
	relay1name: "Zone Name",
	opmode: 0,
    state: 0,
    manualsetpoint: 21,
    mode: 0
};

var schedule = {};

var day1 = [{
    s: 0,
    e: 600,
    sp: 0
}, {
    s: 600,
    e: 900,
    sp: 1800
}, {
    s: 900,
    e: 1700,
    sp: 1200
}, {
    s: 1700,
    e: 2200,
    sp: 0
}, {
    s: 2200,
    e: 2400,
    sp: 800
}];

schedule['mon'] = JSON.parse(JSON.stringify(day1));
schedule['tue'] = JSON.parse(JSON.stringify(day1));
schedule['wed'] = JSON.parse(JSON.stringify(day1));
schedule['thu'] = JSON.parse(JSON.stringify(day1));
schedule['fri'] = JSON.parse(JSON.stringify(day1));
schedule['sat'] = JSON.parse(JSON.stringify(day1));
schedule['sun'] = JSON.parse(JSON.stringify(day1));

//schedule = server_get2("thermostat_schedule"); //all data * 100 to avoid floating point on the ESP8266 side
for (var d in schedule) {
    for (var z in schedule[d]) {
        schedule[d][z].s /= 100;
        schedule[d][z].e /= 100;
        schedule[d][z].sp /= 100;
    }
}

var maxc = 1;
var minc = 0;
// ================================================
// State variables
// ================================================
var editmode = 'move';
$("#mode-move").css("background-color", "#ff9600");
var key = 1;
var day = "mon";
var mousedown = 0;
var slider_width = $(".slider").width();
var slider_height = $(".slider").height();
var changed = 0;

state = server_get2("state");

update();
updateclock();
setInterval(server_get, 5000);
setInterval(updateclock, 1000);

$("#relay1").click(function () {

    $(".thermostatmode").css("background-color", "#555");
    $("#manual_thermostat").css("background-color", "#ff9600");
    thermostat.mode = 0;

    save("thermostat_mode", (thermostat.mode).toString());    
    
    thermostat.relay1state++;
	if (thermostat.relay1state > 1) thermostat.relay1state = 0;

    if (thermostat.relay1state==1) {
        $(this).html("ON");
        $(this).css("background-color", "#ff9600");
    }
    else {
        $(this).html("OFF");
        $(this).css("background-color", "#555");
    }

    save("relay1state", thermostat.relay1state);
});


function updateclock() {
    now = new Date();
    timenow = now.getHours() + (now.getMinutes() / 60);
    today = days[now.getDay()];
	
	checkVisibility();

    $("#datetime").html(today.toUpperCase() + " " + format_time(timenow));
			
    var current_key = 0;
    for (var z in schedule[today]) {
        if (schedule[today][z].s <= timenow && schedule[today][z].e > timenow) {
            if (state.mode == 1) {
                setpoint = schedule[today][z].sp * 1;
                $(".zone-setpoint").html(setpoint.toFixed(1) + "&deg;C");
                current_key = z;
            }
        }

    }

    var sx = $(".slider[day=" + today + "]")[0].offsetLeft;
    var y = $(".slider[day=" + today + "]")[0].offsetTop;
    var x1 = sx + slider_width * (timenow / 24.0);
    var x2 = sx + slider_width * (schedule[today][current_key].s / 24.0);

    x2 = sx;
    $("#timemarker").css('top', y + "px");
    $("#timemarker").css('left', x2 + "px");
    $("#timemarker").css('width', (x1 - x2) + "px");

}

function setStatus(msg,dur,pri){	 // show msg on status bar
		if(statusMsg == true){return};
		statusMsg= true;
		if(pri>0){
			$("#statusView").toggleClass("statusViewAlert",true);
			$("#statusView").toggleClass("statusView",false);
		} else {
			$("#statusView").toggleClass("statusView",true);
			$("#statusView").toggleClass("statusViewAlert",false);
		}
		$("#statusView").show();
		$("#statusView").html(msg);
		dur = dur*1000;
		if(dur >0){
			setTimeout(function(){$("#statusView").hide(200);$("#statusView").html(""); statusMsg= false},dur)
		}
	}

function update() {

	$(".zone-title").html(state.relay1name);
	    
	if (state.relay1 == 1) {
		$("#relay1").html("ON");
		$("#relay1").css("background-color", "#ff9600");
	} else {
		$("#relay1").html("OFF");
		$("#relay1").css("background-color", "#555");
	}
	
	if (state.state === 1) {
		$("#toggle").html("ON");
		$("#toggle").css("background-color", "#ff9600");
	} else {
		$("#toggle").html("OFF");
		$("#toggle").css("background-color", "#555");
	}
	
	if (state.mode === 0) {
		$(".thermostatmode").css("background-color", "#555");
		$("#manual_thermostat").css("background-color", "#ff9600");
		$("#scheduled_thermostat").css("background-color", "#555");
	} else {
		$(".thermostatmode").css("background-color", "#555");
		$("#manual_thermostat").css("background-color", "#555");
		$("#scheduled_thermostat").css("background-color", "#ff9600");
	}
	
	
}
	
$("#toggle").click(function () {
    thermostat.state++;
    if (thermostat.state > 1) thermostat.state = 0;
    if (thermostat.state == 1) {
        $("#toggle").html("ON");
        $(this).css("background-color", "#ff9600");
    }
    else {
        $("#toggle").html("OFF");
        $(this).css("background-color", "#555");
    }

    //save("tx/heating",thermostat.state+","+parseInt(setpoint*100));
    save("thermostat_state", thermostat.state.toString());
});


// ============================================
// SCHEDULER

for (day in schedule) draw_day_slider(day);

function draw_day_slider(day) {
    var out = "";
    var key = 0;
    for (var z in schedule[day]) {
        var left = (schedule[day][z].s / 24.0) * 100;
        var width = ((schedule[day][z].e - schedule[day][z].s) / 24.0) * 100;
        var color = color_map(schedule[day][z].sp);

        out += "<div class='slider-segment' style='left:" + left + "%; width:" + width + "%; background-color:" + color + "' key=" + key + " title='" + (schedule[day][z].sp == 0 ? "Off" : "On") + "'></div>";

        if (key > 0) {
            out += "<div class='slider-button' style='left:" + left + "%;' key=" + key + "></div>";
        }
        key++;
    }
    out += "<div class='slider-label'>" + day.toUpperCase() + "</div>";
    $(".slider[day=" + day + "]").html(out);
}


$("body").on("mousedown", ".slider-button", function (e) {
    mousedown = 1;
    key = $(this).attr('key');
});
$("body").mouseup(function (e) {
    mousedown = 0;
    if (changed) {
        save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
        changed = 0;
    }
});

$("body").on("mousemove", ".slider", function (e) {
    if (mousedown && editmode == 'move') {
        day = $(this).attr('day');
        slider_update(e);
    }
});

$("body").on("touchstart", ".slider-button", function (e) {
    mousedown = 1;
    key = $(this).attr('key');
});
$("body").on("touchend", ".slider-button", function (e) {
    mousedown = 0;
    if (changed) {
        save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
        changed = 0;
    }
});

$("body").on("touchmove", ".slider", function (e) {

    var event = window.event;
    e.pageX = event.touches[0].pageX;
    if (mousedown && editmode == 'move') {
        day = $(this).attr('day');
        slider_update(e);
    }
});

// MERGE
$("body").on("click", ".slider-button", function () {
    if (editmode == 'merge') {
        day = $(this).parent().attr("day");
        key = parseInt($(this).attr("key"),10);
        schedule[day][key - 1].e = schedule[day][key].e;
        schedule[day].splice(key, 1);
        draw_day_slider(day);
        //editmode = 'move';
        save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
    }
});

$("body").on("click", ".slider-segment", function (e) {

    day = $(this).parent().attr("day");
    key = parseInt($(this).attr("key"),10);

    if (editmode == 'split') {
        var x = e.pageX - $(this).parent()[0].offsetLeft;
        var prc = x / slider_width;
        var hour = prc * 24.0;
        hour = Math.round(hour / 0.5) * 0.5;

        if (hour > schedule[day][key].s + 0.5 && hour < schedule[day][key].e - 0.5) {
            var end = parseFloat(schedule[day][key].e);
            schedule[day][key].e = hour;

            schedule[day].splice(key + 1, 0, {
                s: hour,
                e: end,
                sp: schedule[day][key].sp
            });

            draw_day_slider(day);
            
            save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
        }
        //editmode = 'move';
    } else if (editmode == 'move') {
        
        if(schedule[day][key].sp>0)
            $("#slider-segment-state1").prop('checked', true);
        else
            $("#slider-segment-state2").prop('checked', true);
        
        $("#slider-segment-start").val(format_time(schedule[day][key].s));
        $("#slider-segment-end").val(format_time(schedule[day][key].e));

        $("#slider-segment-block").show();
        $("#slider-segment-block-movepos").hide();
    }
});

function slider_update(e) {
    $("#slider-segment-block-movepos").show();
    $("#slider-segment-block").hide();

    if (key !== undefined) {
        var x = e.pageX - $(".slider[day=" + day + "]")[0].offsetLeft;

        var prc = x / slider_width;
        var hour = prc * 24.0;
        hour = Math.round(hour / 0.5) * 0.5;

        if (hour > schedule[day][key - 1].s && hour < schedule[day][key].e) {
            schedule[day][key - 1].e = hour;
            schedule[day][key].s = hour;
            update_slider_ui(day, key);
            changed = 1;
        }
        $("#slider-segment-time").val(format_time(schedule[day][key].s));
    }
    // $("#average_temperature").html(calc_average_schedule_temperature().toFixed(1));


}

$("body").on("click", "#slider-segment-ok", function () {

    if ( $("#slider-segment-state1").is(':checked')) 
        schedule[day][key].sp = 1;
    else
        schedule[day][key].sp = 0;
    
    var color = color_map(schedule[day][key].sp);
    $(".slider[day=" + day + "]").find(".slider-segment[key=" + key + "]").css("background-color", color);

    var time = decode_time($("#slider-segment-start").val());
    if (time != -1 && key > 0 && key < schedule[day].length) {
        if (time >= (schedule[day][key - 1].s + 0.5) && time <= (schedule[day][key].e - 0.5)) {
            schedule[day][key - 1].e = time;
            schedule[day][key].s = time;
        }
    }
    $("#slider-segment-start").val(format_time(schedule[day][key].s));
    update_slider_ui(day, key);

    time = decode_time($("#slider-segment-end").val());
    if (time != -1 && key > 0 && key < (schedule[day].length - 1)) {
        if (time >= (schedule[day][key].s + 0.5) && time <= (schedule[day][key + 1].e - 0.5)) {
            schedule[day][key].e = time;
            schedule[day][key + 1].s = time;
        }
    }
    $("#slider-segment-end").val(format_time(schedule[day][key].e));
    update_slider_ui(day, key + 1);
    save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
    updateclock();

});

$("#slider-segment-movepos-ok").click(function () {

    var time = decode_time($("#slider-segment-time").val());
    if (time != -1 && key > 0) {
        if (time >= (schedule[day][key - 1].s + 0.5) && time <= (schedule[day][key].e - 0.5)) {
            schedule[day][key - 1].e = time;
            schedule[day][key].s = time;
        }
    }
    $("#slider-segment-time").val(format_time(schedule[day][key].s));
    update_slider_ui(day, key);
    save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
});

$("#mode-split").click(function () {
    editmode = 'split';
    $(".editmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
});


$("#mode-move").click(function () {
    editmode = 'move';
    $(".editmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
});

$("#mode-merge").click(function () {
    editmode = 'merge';
    $(".editmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
});

$("#manual_thermostat").click(function () {
    $(".thermostatmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
    thermostat.mode = 0;
	
	setpoint = thermostat.manualsetpoint;
	$(".zone-setpoint").html(setpoint.toFixed(1) + "&deg;C");

    save("thermostat_mode", (thermostat.mode).toString());
    updateclock();
});

$("#scheduled_thermostat").click(function () {
    $(".thermostatmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
    thermostat.mode = 1;
    save("thermostat_mode", (thermostat.mode).toString());
    updateclock();
});

function color_map(temperature) {
	if(temperature>0) 
       return "rgb(200,20,0)";
    return "rgb(200,200,200)";
}

function update_slider_ui(day, key) {
    if (schedule[day] !== undefined && key < schedule[day].length) {
        var slider = $(".slider[day=" + day + "]");
        if (key > 0) {
            var width = ((schedule[day][key - 1].e - schedule[day][key - 1].s) / 24.0) * 100;
            slider.find(".slider-segment[key=" + (key - 1) + "]").css("width", width + "%");
        }

        var left = (schedule[day][key].s / 24.0) * 100;
        var width = ((schedule[day][key].e - schedule[day][key].s) / 24.0) * 100;
        slider.find(".slider-segment[key=" + key + "]").css("width", width + "%");
        slider.find(".slider-segment[key=" + key + "]").css("left", left + "%");
        slider.find(".slider-button[key=" + key + "]").css("left", left + "%");
    }
}

function format_time(time) {
    var hour = Math.floor(time);
    var mins = Math.round((time - hour) * 60);
    if (mins < 10) mins = "0" + mins;
    return hour + ":" + mins;
}

function decode_time(timestring) {
    var time = -1;
    if (timestring.indexOf(":") != -1) {
        var parts = timestring.split(":");
        var hour = parseInt(parts[0],10);
        var mins = parseInt(parts[1],10);

        if (mins >= 0 && mins < 60 && hour >= 0 && hour < 25) {
            if (hour == 24 && mins !== 0) {} else {
                time = hour + (mins / 60);
            }
        }
    }
    return time;
}

function calc_average_schedule_temperature() {
    var sum = 0;
    for (var d in schedule) {
        for (var z in schedule[d]) {
            var hours = (schedule[d][z].e - schedule[d][z].s)
            sum += (schedule[d][z].sp * hours);
        }
    }
    return sum / (24 * 7.0);
}

function calc_schedule_esp(sched) {
    var fixsched = JSON.parse(JSON.stringify(sched));
    for (var d in fixsched) {
        fixsched[d].s *= 100;
        fixsched[d].e *= 100;
        fixsched[d].sp *= 100;
    }
    return fixsched;
}

// function for checking if the page is visible or not
// (if not visible it will stop updating data)
function checkVisibility() {
    $(window).bind("focus", function(event) {
        visibleFlag = 1;
    });

    $(window).bind("blur", function(event) {
        visibleFlag = 0;
    });
}

function save(param, payload) {
    $.ajax({
        type: 'POST',
        url: "thermostat.cgi?param=" + param,
        data: payload,
		dataType: 'text',
		cache: false,
        async: true,
		timeout: 5000,
		success: function (data) {
			statusMsg = false;
			if(!connected) setStatus("Connected",2,0); 
			connected=true;
		},
		error: function (XMLHttpRequest, textStatus, errorThrown) {
			if(connected) setStatus("No connection to server!",0,1);
			connected=false;
		}
    });
}

function server_get() {
    var output = {};
	if (visibleFlag) {
		$.ajax({
			url: "thermostat.cgi?param=state",
			dataType: 'json',
			async: true,
			timeout: 3000,
			success: function (data) {
				if (data.length !== 0) {
					statusMsg = false;
					if(!connected) setStatus("Connected",2,0); 
					connected=true;
					output = data;
					thermostat=data;
					thermostat.manualsetpoint/=100;
					update();
				}
			},
			error: function (data) {
				if(connected) setStatus("No connection to server!",0,1);
				connected=false;
			}
		});
	}
    return output;
}

function server_get2(param) {
    var output = {};
	if (visibleFlag) {
		$.ajax({
			url: "thermostat.cgi?param=" + param,
			dataType: 'json',
			async: false,
			timeout: 3000,
			success: function (data) {
				if (data.length !== 0) output = data;
					statusMsg = false;
					if(!connected) setStatus("Connected",2,0); 
					connected=true;
			},
			error: function (data) {
				if(connected) setStatus("No connection to server!",0,1);
				connected=false;
			}
		});
	}
    return output;
}
