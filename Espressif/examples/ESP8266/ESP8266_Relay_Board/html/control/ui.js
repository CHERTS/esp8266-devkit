/*
Based on Luca Soltoggio's work
http://arduinoelettronica.wordpress.com/

*/

var visibleFlag = 1;

// get and parse json data from ESP8266 and update HTML page
function get_esp_data() {
    if (visibleFlag) {
        var jqxhr = $.getJSON('state.cgi?random=' + Math.random(), function(data) {
            // call value_update function passing json variable
            value_update(data);
            $("[data-role='slider']").slider('enable');
            $('label').css('color','#333');
            $('.ui-bar-d').css('color', '#333');
            $('.inputvalue').css('color','coral');
            $('.s-title').css('color','#333');
        })
        .error(function() {
            $("[data-role='slider']").slider('disable');
            $('label').css('color','#BBBBBB');
            $('.ui-bar-d').css('color', '#BBBBBB');
            $('.inputvalue').css('color','#BBBBBB');
            $('.s-title').css('color','#BBBBBB');
        });
    }
}


function set_esp_data(pname,pvalue) {
    $.getJSON('relay.cgi?' + pname + '=' + pvalue + '&random=' + Math.random(), function(data) {
        value_update(data);
    });
}

// if a slider changed, this funciont will call the function above 
function detect_changes() {
    $("[data-role='slider']").change(function() {
        set_esp_data($(this).attr("name"),$(this).val());
    });
}

// function for parsing json data and updating HTML page
// this will update slider status and label value
function value_update(data) {
    $.each(data, function (index, value) {
	if (index.match(/relay.*/)) {
            $('#'+index).val(value).slider("refresh");
        }
        else
        {
            $('#'+index).text(value);
        }
    });
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

// start all above every 5 seconds
$(document).ready(function(){
    get_esp_data();
    detect_changes();
    setInterval('get_esp_data()', 5000);
    checkVisibility();
});
