/*
Copyright (c) 2014 lonely-pixel.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

var Picker = function (options) {

    /* default settings */

    this.settings = {

        parent: document.body,
        orientation: 'right',
        x: 'auto',
        y: 'auto',
        arrow_size: 20
    };

    if (options instanceof HTMLElement) {

        this.settings.parent = options;

    } else {

        for (var name in options) {

            this.settings[name] = options[name];
        }
    }

    /* slider variables and settings */

    this.sliders = {

        'picker_selector': {
            down: false
        },
        'picker_hue': {
            down: false,
            vertical: true
        },
        'picker_opacity': {
            down: false,
            vertical: true
        },
    };

    /* colour storage and conversion */

    this.colour = this.color = {

        hue: 0,
        saturation: 1,
        value: 1,
        alpha: 1,

        /* convert to HSL */

        hsl: function () {

            var h = this.hue;
            var l = (2 - this.saturation) * this.value;
            var s = this.saturation * this.value;
            s /= l <= 1 ? l : 2 - l;
            l /= 2;

            s *= 100;
            l *= 100;

            return {
                h: h,
                s: s,
                l: l,
                toString: function () {

                    return 'hsl(' + this.h + ', ' + this.s + '%, ' + this.l + '%)';
                }
            };
        },

        /* convert to HSLA */

        hsla: function () {

            var hsl = this.hsl();

            hsl.a = this.alpha;

            hsl.toString = function () {

                return 'hsla(' + this.h + ', ' + this.s + '%, ' + this.l + '%, ' + this.a + ')';
            };

            return hsl;
        },

        /* convert to RGB */

        rgb: function () {

            var r, g, b;

            var h = this.hue;
            var s = this.saturation;
            var v = this.value;

            h /= 60;

            var i = Math.floor(h);
            var f = h - i;
            var p = v * (1 - s);
            var q = v * (1 - s * f);
            var t = v * (1 - s * (1 - f));

            r = [v, q, p, p, t, v][i];
            g = [t, v, v, q, p, p][i];
            b = [p, p, t, v, v, q][i];

            return {
                r: Math.floor(r * 255),
                g: Math.floor(g * 255),
                b: Math.floor(b * 255),
                toString: function () {

                    return 'rgb(' + this.r + ', ' + this.g + ', ' + this.b + ')';
                }
            };
        },

        /* convert to RGBA */

        rgba: function () {

            var rgb = this.rgb()

            rgb.a = this.alpha;

            rgb.toString = function () {

                return 'rgba(' + this.r + ', ' + this.g + ', ' + this.b + ', ' + this.a + ')';
            };

            return rgb;
        },

        /* convert to hex */

        hex: function () {

            var rgb = this.rgb();

            function to_hex(c) {

                var hex = c.toString(16);

                return hex.length == 1 ? '0' + hex : hex;
            }

            return {
                r: to_hex(rgb.r),
                g: to_hex(rgb.g),
                b: to_hex(rgb.b),
                toString: function () {

                    return '#' + this.r + this.g + this.b;
                }
            }
        }
    };

    /* event functions */

    this.on_done = null;
    this.on_change = null;
};

/******************************************************

CSS applied to Picker's elements.

You can customise it here or dynamically via Javascript.
e.g your_picker_object.css.wrapper.padding = '20px'

******************************************************/

Picker.prototype.css = {

    wrapper: {
        selector: '#picker_wrapper',
        background: '#f2f2f2',
//        position: 'absolute',
        whiteSpace: 'nowrap',
        padding: '10px',
        cursor: 'default',
        fontFamily: 'sans-serif',
        fontWeight: '100',
        display: 'inline-block',
        boxShadow: '0 0 10px 1px rgba(0,0,0,0.4)',
        overflow: 'visible',
        textAlign: 'left',
        fontSize: '16px'
    },

    arrow: {
        selector: '#picker_arrow',
        height: '0',
        width: '0',
        borderLeft: '20px solid transparent',
        borderRight: '20px solid transparent',
        borderBottom: '20px solid #f2f2f2',
        position: 'absolute',
        top: '-20px',
        left: '0'
    },

    colour_picker: {
        selector: '#picker_selector',
        width: '180px',
        height: '150px',
        position: 'relative',
        background: 'hsl(0, 100%, 50%)',
        display: 'inline-block',
        border: '1px solid #ccc'
    },

    saturation_overlay: {

        selector: '#picker_saturation',
        width: '180px',
        height: '150px',
        position: 'absolute',
        background: 'url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALQAAACWCAYAAAB3qaIPAAADB0lEQVR4Ae3SQQrCMABE0cb7n9nYIgUXIW7ExecJUkmlOjNvzDmfx3HM831fPz/fZ/d1dW91tvv+7t4vn/Wv3/Gf335WPazOdrvs7n191hhjPk7IXhrINAB0ZkpBrgaA5iDVANCpOYUBmoFUA0Cn5hQGaAZSDQCdmlMYoBlINQB0ak5hgGYg1QDQqTmFAZqBVANAp+YUBmgGUg0AnZpTGKAZSDUAdGpOYYBmINUA0Kk5hQGagVQDQKfmFAZoBlINAJ2aUxigGUg1AHRqTmGAZiDVANCpOYUBmoFUA0Cn5hQGaAZSDQCdmlMYoBlINQB0ak5hgGYg1QDQqTmFAZqBVANAp+YUBmgGUg0AnZpTGKAZSDUAdGpOYYBmINUA0Kk5hQGagVQDQKfmFAZoBlINAJ2aUxigGUg1AHRqTmGAZiDVANCpOYUBmoFUA0Cn5hQGaAZSDQCdmlMYoBlINQB0ak5hgGYg1QDQqTmFAZqBVANAp+YUBmgGUg0AnZpTGKAZSDUAdGpOYYBmINUA0Kk5hQGagVQDQKfmFAZoBlINAJ2aUxigGUg1AHRqTmGAZiDVANCpOYUBmoFUA0Cn5hQGaAZSDQCdmlMYoBlINQB0ak5hgGYg1QDQqTmFAZqBVANAp+YUBmgGUg0AnZpTGKAZSDUAdGpOYYBmINUA0Kk5hQGagVQDQKfmFAZoBlINAJ2aUxigGUg1AHRqTmGAZiDVANCpOYUBmoFUA0Cn5hQGaAZSDQCdmlMYoBlINQB0ak5hgGYg1QDQqTmFAZqBVANAp+YUBmgGUg0AnZpTGKAZSDUAdGpOYYBmINUA0Kk5hQGagVQDQKfmFAZoBlINAJ2aUxigGUg1AHRqTmGAZiDVANCpOYUBmoFUA0Cn5hQGaAZSDQCdmlMYoBlINQB0ak5hgGYg1QDQqTmFAZqBVANAp+YUBmgGUg0AnZpTGKAZSDUAdGpOYYBmINUA0Kk5hQGagVQDQKfmFAZoBlINAJ2aUxigGUg1AHRqTmGAZiDVANCpOYUBmoFUAy+AUbcs6wwU4wAAAABJRU5ErkJggg==)'
    },

    value_overlay: {

        selector: '#picker_value',
        width: '180px',
        height: '150px',
        position: 'absolute',
        background: 'url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALQAAACWCAYAAAB3qaIPAAADFUlEQVR4Ae3WgQ3AMAzDsHbY/ycPa95QmQ9kC0bWchIIJbAPyxPigXJ5Au/hH6mdBBIJjNAWOlEliEmA0DxIJeDlSNUJxkJzIJUAoVN1giE0B1IJ+KFTdYKx0BxIJUDoVJ1gvBwcSCVgoVN1giE0B1IJeDlSdYKx0BxIJUDoVJ1gCM2BVAJ+6FSdYCw0B1IJEDpVJxgvBwdSCVjoVJ1gCM2BVAKETtUJxg/NgVQCFjpVJxhCcyCVgJcjVScYC82BVAKETtUJxsvBgVQCFjpVJxhCcyCVAKFTdYLxQ3MglYCFTtUJhtAcSCXg5UjVCcZCcyCVAKFTdYIhNAdSCfihU3WCsdAcSCVA6FSdYLwcHEglYKFTdYIhNAdSCXg5UnWCsdAcSCVA6FSdYAjNgVQCfuhUnWAsNAdSCRA6VScYLwcHUglY6FSdYAjNgVQChE7VCcYPzYFUAhY6VScYQnMglYCXI1UnGAvNgVQChE7VCcbLwYFUAhY6VScYQnMglQChU3WC8UNzIJWAhU7VCYbQHEgl4OVI1QnGQnMglQChU3WCITQHUgn4oVN1grHQHEglQOhUnWC8HBxIJWChU3WCITQHUgkQOlUnGD80B1IJWOhUnWAIzYFUAl6OVJ1gLDQHUgkQOlUnGC8HB1IJWOhUnWAIzYFUAoRO1QnGD82BVAIWOlUnGEJzIJWAlyNVJxgLzYFUAoRO1QmG0BxIJeCHTtUJxkJzIJUAoVN1gvFycCCVgIVO1QmG0BxIJeDlSNUJxkJzIJUAoVN1giE0B1IJ+KFTdYKx0BxIJUDoVJ1gvBwcSCVgoVN1giE0B1IJEDpVJxg/NAdSCVjoVJ1gCM2BVAJejlSdYCw0B1IJEDpVJxgvBwdSCVjoVJ1gCM2BVAKETtUJxg/NgVQCFjpVJxhCcyCVgJcjVScYC82BVAKETtUJhtAcSCXgh07VCcZCcyCVAKFTdYLxcnAglYCFTtUJhtAcSCXg5UjVCcZCcyCVAKFTdYIhNAdSCfihU3WCsdAcSCVA6FSdYLwcHEglsA/NlyICc3UCP/PSBEmeyvh+AAAAAElFTkSuQmCC)'
    },

    hue_slider: {

        selector: '#picker_hue',
        width: '16px',
        height: '150px',
        position: 'relative',
        display: 'inline-block',
        marginLeft: '10px',
        background: 'url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAACWCAYAAADe8ajeAAABUklEQVRoBe2ZwQ4CMQhEIXJR//9T1biXyp1DJT4SzOKxRcLM0AJdXWJLwJ+J3EF3IhUOb+0jxDkcyD9qXpGHJ1R58nDyMMtA/6OnS95oGdVDFurQnlnWN/bjcEPQF9vD4RckbUzssTHIbtsr+4+NPa9yf8j9I8Tvw/6Q8QhxDvGj1x6yCl3o5bq5j5LbJmzP7hP9OExqEMwLOGSHUVe5fx72h9w/Qvws94eMR4hziB+9/pBxDmGHKgc70ausg30iEGHbJX+3wR2yLWdFhAM5dCu5BRdlVM5RFqwrEntECTTnFipEmdsmp0Gw9jLKfgFXd8fWZbgTEevvEO43xeCOuIDD/pDxCPE8xFXuDxnncByGGpFd+IMLdlTOihrs1ed5ttDDc4+/wbLtoTuEv9DYGSHjorCTWYXK/SHjHI7DcGMmFwouhxElqUEwP6Eoui5sof8A9nlhGxrI9UUAAAAASUVORK5CYII=)',
        border: '1px solid #ccc'
    },

    opacity_slider: {

        selector: '#picker_opacity',
        width: '16px',
        height: '150px',
        position: 'relative',
//        display: 'inline-block',
		display: 'none',
        marginLeft: '10px',
        background: '#f00',
        border: '1px solid #ccc'
    },

    opacity_slider_overlay: {

        selector: '#picker_opacity_fade',
        width: '16px',
        height: '150px',
//        display: 'inline-block',
		display: 'none',
        position: 'absolute',
        background: 'url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAACWCAYAAADXGgikAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAEBklEQVRo3q1ay6HUQAyz8+ZKIZzTAn3QB4VQB4Vs6qAMcdnAZCLJDvAu7C/J2JZl2SbiH//y9XptEYGIyOU7RETu+36+/n3N/KMREdt8wfR6Izecb4KIyDFdlPMX7LTs/SBPSnKiUJ+N6cnsByCnwmzGfIIUtsb7NyCm5Ji+TOE8a1oC+LR6dr7qOA4WiWRhzOX4QT5fT3uLQhbHjyXswXwQBR4uETrDyFDHbnxzKDMB6zGJCWBhjAVUYVCYDIkUaeLzP1A+jmMzjjvTeUXjDYlVGOX3w0AXDbMoDlhOQEVoTed05NExAQ0/RIdQIIAT1QkqE26ITQCf3RHffMCSjZoQDW6MlQ8+TJzX9zduZCeQ9jLOHAYwsVyEpYpRVmY3mr/P9bVyYkUol2zMZjmjp5z5gD5p33eJAcaJigMoBhQnuvIOVRc6hEL9MIqnsahQEyoOhArlMNB1Ja/0wWYKClY++NLgg+gwEtRTHEodobBaeaP/lVC6SiVVMkVhRiqBoRCXTmApgRFP8mM0dKGq2hc+kJS277ut0KtCUeShkqtd2qT4GELiZcEPti4owZEMZMPJWJInsmfCw3bngkQnacOgFGe/8NVx4XEccJSnkKhQaBuOyg+SD1J0JQqByZxoK7BLtlHEutSNQ3BAGL14A9KH6gUa72Mcx5GCNLH0CzSx5qYLBtZQ7fHpxFRQrUxhhJLN7sUiURUSKnU3Qxhp/LGp2pgPkivVEKb7hwTwTdTDWR+w6Yal9c3goCWyUDSkpcgKw4PJkKgycCP9wvyvHURV0t/KPBiVEqo640GF2li/QEG08EEyTswH1RhOI3U54BKlihNL7TTMILKTXBhmglOVtmCVCcXR0RVZ7SlOAvjumsxJH1Az3TRPlfoLKkdB5VmZ4wbT2UBmVIPprDJzLW0VH9zgrhQKuppZlbatMZCLkw8UlZ/9guKC7CCxzM5hMIAGT/zuG13GKQFWFpaWOcOM/lsspfQBTCNGFYpzVLobJIAf7pjTPFHOVDvduwx1NUuruDI7kyyl3qGKa2dNdBuJKhS6rnZjk6xK5ubf+kDO1mZ9QLXye35QjkSr8WeKzJRD2apCJ+udwwzpU62RHBLdlD+dWmeDJygtpZBYLTDL1tcl1WVgmQBe/2OeiILWW41nmIZLNuTdjWd0mq54aEY4jdR5OpQTO51re8fSmh/YbHzrAxkN1y+EUCqPFxSWI7ubrnDDOCUwWKmXPqiOmx2p+2g1Us3SWANOt18J4KeZI618cPuNS6bWfyUYZobmeDHCLOuyGAtt3TVRq9SpdG6lspsrpwkdlboqpcuV+tkvyE3PtF+IipXDVGQbxq1oOl2pt7Wxsl/uF8L1yswHGX4h5fYL+Qv+btsS2cFCRgAAAABJRU5ErkJggg==)'
    },

    colour_selector: {

        selector: '.picker_selector',
        width: '10px',
        height: '10px',
        position: 'absolute',
        display: 'inline-block',
        borderRadius: '20px',
        cursor: 'pointer',
        border: '2px solid #fff',
        boxShadow: '0 0 3px 1px #67b9ff',
        background: '#f00',
        left: '173px',
        top: '-7px'
    },

    slider_bar: {

        selector: '.picker_slider_bar',
        width: '100%',
        height: '10px',
        position: 'absolute',
        top: '-7px',
        borderRadius: '2px',
        cursor: 'pointer',
        border: '2px solid #fff',
        boxShadow: '0 0 3px 1px #67b9ff',
        marginLeft: '-2px',
        background: '#f00',
        fontSize: '16px'
    },

    sample: {

        selector: '#picker_sample',
        width: '180px',
        height: '24px',
        background: 'url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAIUlEQVR42mM4c+bMf3yYEGAYNWBYGEBIASELRg0YFgYAAMoUr64OfmpAAAAAAElFTkSuQmCC)',
        display: 'inline-block',
        position: 'relative',
        marginTop: '10px',
        border: '1px solid #aaa',
    },

    sample_colour: {

        selector: '#picker_sample_colour',
        width: '100%',
        height: '100%',
        background: 'rgba(255,0,0,0.4)',
        position: 'absolute',

    },

    done_button: {

        selector: '#picker_done',
        width: '54px',
        height: '22px',
        lineHeight: '22px',
        background: '#e2e2e2',
//        display: 'inline-block',
		display: 'none',
        border: '1px solid #ccc',
        marginLeft: '10px',
        textAlign: 'center',
        color: '#aaa',
        position: 'absolute',
        right: '12px',
        bottom: '15px',
        cursor: 'pointer',
        boxShadow: '0 0 3px 1px #eee'
    }
};

/******************************************************

Apply the CSS to Picker's elements.

You need to call this if you customise the CSS after
you call show().

******************************************************/

Picker.prototype.apply_style = function () {

    for (var name in this.css) {

        var element = this.css[name];

        var tags = document.querySelectorAll(element.selector);

        if (!tags.length) continue;

        var i = tags.length;

        while (i--) {

            var tag = tags[i];

            for (var name in element) {

                if (name == 'selector') continue;

                var property = element[name];

                tag.style[name] = property;
            }
        }
    }
};

/********************************

Show or create the picker HTML.

********************************/

Picker.prototype.show = function () {

    /* unhide html if it exists */

    var wrapper = document.getElementById('picker_wrapper');

    if (wrapper) {

        wrapper.style.display = 'inline-block';

        return;
    }

    /* append new html */

    var html = '';

    html += '<div id="picker_wrapper">';
    html += '   <div id="picker_arrow"></div>';
    html += '   <div id="picker_selector">';
    html += '       <div id="picker_saturation"></div>';
    html += '       <div id="picker_value"></div>';
    html += '       <div class="picker_selector"></div>';
    html += '   </div>';
    html += '   <div id="picker_hue" class="picker_slider">';
    html += '       <div class="picker_slider_bar"></div>';
    html += '   </div>';
    html += '   <div id="picker_opacity" class="picker_slider">';
    html += '       <div id="picker_opacity_fade"></div>';
    html += '       <div class="picker_slider_bar"></div>';
    html += '   </div>';
    html += '   <br>';
    html += '   <div id="picker_sample">';
    html += '   <div id="picker_sample_colour"></div>';
    html += '   </div>';
    html += '   <div id="picker_done">ok</div>';
    html += '</div>';

    var parent = this.settings.parent;
    
    if(parent.style.position != 'absolute') {
        
        parent.style.position = 'relative';
    }

    parent.innerHTML += html;

    /* apply css */

    this.apply_style();

    /* set positioning */

    var wrapper = document.getElementById('picker_wrapper');
    var arrow = document.getElementById('picker_arrow');

    if (this.settings.x === 'auto') {

        switch (this.settings.orientation) {

        case 'left':
            wrapper.style.left = -wrapper.offsetWidth - this.settings.arrow_size - 4 + 'px';
            break;

        case 'top':
        case 'bottom':
            wrapper.style.left = parent.offsetWidth / 2 - this.settings.arrow_size + 'px';
            break;

        case 'center':
        case 'centre':
            wrapper.style.left = -wrapper.offsetWidth / 2 + parent.offsetWidth / 2 + 'px';
            break;

        default:
            wrapper.style.left = parent.offsetWidth + this.settings.arrow_size + 4 + 'px';
            break;
        }

    } else {

        wrapper.style.left = parseInt(this.settings.x) + 'px';
    }

    if (this.settings.y === 'auto') {

        switch (this.settings.orientation) {

        case 'top':
            wrapper.style.top = -wrapper.offsetHeight - this.settings.arrow_size - 4 + 'px';
            break;

        case 'bottom':
            wrapper.style.top = parent.offsetHeight + this.settings.arrow_size + 4 + 'px';
            break;

        case 'center':
        case 'centre':
            wrapper.style.top = -parent.offsetHeight / 2 + -wrapper.offsetHeight / 2 + 'px';
            break;

        default:
            wrapper.style.top = parent.offsetHeight / 2 - this.settings.arrow_size + 'px';
            break;
        }

    } else {

        wrapper.style.top = parseInt(this.settings.y) + 'px';
    }

    /* set arrow position */

    switch (this.settings.orientation) {

    case 'left':
        arrow.style.borderLeft = this.settings.arrow_size + 'px solid' + this.css.wrapper.background;
        arrow.style.borderRight = 'none';
        arrow.style.borderTop = this.settings.arrow_size + 'px solid transparent';
        arrow.style.borderBottom = this.settings.arrow_size + 'px solid transparent';

        arrow.style.top = '0';
        arrow.style.right = -this.settings.arrow_size + 'px';
        arrow.style.left = '';
        break;

    case 'top':
        arrow.style.borderLeft = this.settings.arrow_size + 'px solid transparent';
        arrow.style.borderRight = this.settings.arrow_size + 'px solid transparent';
        arrow.style.borderTop = this.settings.arrow_size + 'px solid' + this.css.wrapper.background;
        arrow.style.borderBottom = 'none';

        arrow.style.bottom = -this.settings.arrow_size + 'px';
        arrow.style.top = '';
        arrow.style.left = '0';
        break;

    case 'bottom':
        arrow.style.borderLeft = this.settings.arrow_size + 'px solid transparent';
        arrow.style.borderRight = this.settings.arrow_size + 'px solid transparent';
        arrow.style.borderTop = 'none';
        arrow.style.borderBottom = this.settings.arrow_size + 'px solid' + this.css.wrapper.background;

        arrow.style.top = -this.settings.arrow_size + 'px';
        arrow.style.left = '0';
        break;

    case 'center':
    case 'centre':
        arrow.style.borderLeft = 'none';
        arrow.style.borderRight = 'none';
        arrow.style.borderTop = 'none';
        arrow.style.borderBottom = 'none';
        break;

    default:
        arrow.style.borderLeft = 'none';
        arrow.style.borderRight = this.settings.arrow_size + 'px solid' + this.css.wrapper.background;
        arrow.style.borderTop = this.settings.arrow_size + 'px solid transparent';
        arrow.style.borderBottom = this.settings.arrow_size + 'px solid transparent';

        arrow.style.top = '0';
        arrow.style.left = -this.settings.arrow_size + 'px';
        break;
    }

    this.bind_events();

    this.update_sample();
};

/*************************

Hides the picker window.

*************************/

Picker.prototype.hide = function () {

    var element = document.getElementById('picker_wrapper');

    if (element) {

        element.style.display = 'none';
    }
};

/*****************************************

Update the saturation and value variables.

*****************************************/

Picker.prototype.update_picker_selector = function (element, x, y) {

    this.colour.saturation = x / (element.offsetWidth - 2);

    this.colour.value = 1 - y / (element.offsetHeight - 2);

    this.update_opacity_slider();

    this.update_sample();

    this.update_picker_slider();
};

/***********************

Update the hue variable.

***********************/

Picker.prototype.update_picker_hue = function (element, x, y) {

    this.colour.hue = (1 - y / (element.offsetHeight - 2)) * 360;

    this.update_selector_hue();

    this.update_sample();

    this.update_hue_slider();
};

/*************************

Update the alpha variable.

*************************/

Picker.prototype.update_picker_opacity = function (element, x, y) {

    this.colour.alpha = 1 - y / (element.offsetHeight - 2);

    this.update_sample();

    this.update_opacity_slider();
};

/*************************************

Update the selected colour sample
The on_change function is called here.

*************************************/

Picker.prototype.update_sample = function () {

    var sample = document.getElementById('picker_sample_colour');

    sample.style.background = this.colour.hsla().toString();

    this.update_opacity_hue();

    if (this.on_change) {

        this.on_change(this.colour);
    }
};

/******************************

Update the colour picker's hue.

******************************/

Picker.prototype.update_selector_hue = function () {

    var picker = document.getElementById('picker_selector');

    picker.style.background = 'hsl(' + this.colour.hue + ', 100%, 50%)';

    this.update_picker_slider();

    this.update_opacity_slider();
};

/************************

Update the opaciy slider.

************************/

Picker.prototype.update_opacity_hue = function () {

    var picker = document.getElementById('picker_opacity');

    picker.style.background = this.colour.hsl().toString();
};

/*******************************

Update the colour picker slider.

*******************************/

Picker.prototype.update_picker_slider = function () {

    var slider = document.querySelector('#picker_selector .picker_selector');

    slider.style.background = this.colour.hsl().toString();
};

/*************************

Update the hue slider bar.

*************************/

Picker.prototype.update_hue_slider = function () {

    var slider = document.querySelector('#picker_hue .picker_slider_bar');

    slider.style.background = 'hsl(' + this.colour.hue + ', 100%, 50%)';
};

/*****************************

Update the opacity slider bar.

*****************************/

Picker.prototype.update_opacity_slider = function () {

    var slider = document.querySelector('#picker_opacity .picker_slider_bar');

    slider.style.background = this.colour.hsla().toString();
};

/***********************

Handle slider movements.

***********************/

Picker.prototype.mouse_move = function (e, element, _this, override) {

    var rect = element.getBoundingClientRect();
    var x = e.clientX - rect.left;
    var y = e.clientY - rect.top;

    if (override || (_this.sliders[element.id] && _this.sliders[element.id].down)) {

        var slider_info = _this.sliders[element.id];

        var slider = element.querySelectorAll('.picker_selector')[0] || element.querySelectorAll('.picker_slider_bar')[0];

        if (!slider) return;

        if (!slider_info.vertical) {

            x = Math.min(Math.max(x - slider.offsetWidth / 2, -(slider.offsetWidth / 2)), element.offsetWidth - slider.offsetWidth / 2 - 2);

            slider.style.left = x + 'px';
        }

        y = Math.min(Math.max(y - slider.offsetHeight / 2, -(slider.offsetHeight / 2)), element.offsetHeight - slider.offsetHeight / 2 - 2);

        slider.style.top = y + 'px';

        if (_this['update_' + element.id]) {

            _this['update_' + element.id](element, x + slider.offsetWidth / 2, y + slider.offsetHeight / 2);
        }
    }
};

/*****************

Bind mouse events.

*****************/

Picker.prototype.bind_events = function () {

    var wrapper = document.getElementById('picker_wrapper');

    var done = document.getElementById('picker_done');

    var colour_select = document.getElementById('picker_selector');
    var hue_select = document.getElementById('picker_hue');
    var opacity_select = document.getElementById('picker_opacity');

    var picker_slider = document.querySelector('#picker_selector .picker_selector');
    var hue_slider = document.querySelector('#picker_hue .picker_slider_bar');
    var opacity_slider = document.querySelector('#picker_opacity .picker_slider_bar');

    var _this = this;

    colour_select.onmousemove = hue_select.onmousemove = opacity_select.onmousemove = function (e) {

        _this.mouse_move(e, this, _this);

        e.preventDefault();
    };

    colour_select.onmousedown = hue_select.onmousedown = opacity_select.onmousedown = function (e) {

        _this.sliders[this.id].down = true;

        _this.mouse_move(e, this, _this, true);

        e.preventDefault();
    };
    picker_slider.onmousedown = hue_slider.onmousedown = opacity_slider.onmousedown = function () {

        _this.sliders[this.parentNode.id].down = true;
    };


    wrapper.onclick = wrapper.onmousedown = function (e) {
        e.stopPropagation();
        e.preventDefault();
        return false;
    };

    document.getElementsByTagName('html')[0].onmouseup = function () {
        for (var name in _this.sliders) {
            _this.sliders[name].down = false;
        }
    };

//    document.getElementsByTagName('html')[0].onclick = function (e) {
//        if (e.target && e.target != _this.settings.parent) {
//            _this.hide();
//        }
//    };

    done.onclick = function () {

        _this.done();
    };
};

/********************************************************

Hides the window when, called when the button is clicked.
The on_done function is called here.

********************************************************/

Picker.prototype.done = function () {

    this.hide();

    if (this.on_done) {

        this.on_done(this.colour);
    }
};