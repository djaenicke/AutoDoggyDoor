var days = ["m", "t", "w", "th", "f", "sat", "sun", "all"];
var intervals = [];
var id_cntr = 0;

function set_trigger()
{
    setTimeout(function() { load_data(); }, 1000);
}

function load_data() {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() { populate_data(xmlhttp); };
    xmlhttp.open('GET', "load_data.cgi", true);
    xmlhttp.send(null);
}

function populate_data(http_request)
{
   if (http_request.readyState == 4 && http_request.status == 200) 
   {
        var obj = JSON.parse(http_request.responseText);
        const element = document.getElementById("zip");
        element.value = obj.zip;
        obj.intervals.forEach(interval => {
            if (interval.hasOwnProperty('id'))
            {
                if (-1 == interval.id)
                {
                    interval.id = id_cntr;
                    interval.start_time = "";
                    interval.stop_time = "";
                }
                add_time_interval(interval);
                id_cntr++;
            }
        })
   }
}

window.onload=set_trigger;

function refresh_rtc_value()
{
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() { rtc_data_rxed(xmlhttp); };
    xmlhttp.open('GET', "rtcdata.cgi", true);
    xmlhttp.send(null);
}

function rtc_data_rxed(http_request)
{
    if (http_request.readyState == 4)
    {
        if (http_request.status == 200)
        {
            const element = document.getElementById("rtc_time");
            element.innerHTML = http_request.responseText;
        }
    }
}

function refresh_weather_value()
{
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() { weather_data_rxed(xmlhttp); };
    xmlhttp.open('GET', "weatherdata.cgi", true);
    xmlhttp.send(null);
}

function weather_data_rxed(http_request)
{
    if (http_request.readyState == 4)
    {
        if (http_request.status == 200)
        {
            const element = document.getElementById("weather_status");
            element.innerHTML = "Weather Status: " + http_request.responseText;
        }
    }
}

function refresh_time_schedule_status()
{
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() { time_schedule_data_rxed(xmlhttp); };
    xmlhttp.open('GET', "ts_status.cgi", true);
    xmlhttp.send(null);
}

function time_schedule_data_rxed(http_request)
{
    if (http_request.readyState == 4)
    {
        if (http_request.status == 200)
        {
            const element = document.getElementById("ts_status");
            element.innerHTML = "Time Schedule Status: " + http_request.responseText;
        }
    }
}

function add_time_interval(new_interval)
{
    var interval = new Object();
    
    interval.id = parseInt(new_interval.id);
    interval.days = parseInt(new_interval.days);
    interval.start_time = new_interval.start_time;
    interval.stop_time = new_interval.stop_time;
    intervals.push(interval);
    
    const div = document.createElement("div");
    div.className = "row";
    div.id = "row_" + interval.id;

    /************************************************************************/
    var temp_div = document.createElement("div");
    temp_div.className = "time";

    var temp_input = document.createElement("input");
    temp_input.className = "time_input";
    temp_input.type = "time";
    temp_input.id = "start_" + interval.id;
    temp_input.addEventListener("input", set_start_time);

    if (interval.start_time !== null)
    {
        temp_input.value = interval.start_time;
    }

    temp_div.appendChild(temp_input);
    div.appendChild(temp_div);
    /************************************************************************/

    /************************************************************************/
    var temp_div = document.createElement("div");
    temp_div.className = "time";

    var temp_input = document.createElement("input");
    temp_input.className = "time_input";
    temp_input.type = "time";
    temp_input.id = "stop_" + interval.id;
    temp_input.addEventListener("input", set_stop_time);

    if (interval.stop_time !== null)
    {
        temp_input.value = interval.stop_time;
    }
    
    temp_div.appendChild(temp_input);
    div.appendChild(temp_div);
    /************************************************************************/

    /************************************************************************/
    var cnt = 0;
    days.forEach(day => {
        var temp_div = document.createElement("div");
        temp_div.className = "day";
        temp_div.name = day;
        
        var temp_input = document.createElement("input");
        temp_input.className = "day_select";
        temp_input.type = "checkbox";
        temp_input.name = day;
        temp_input.id = day + '_' + interval.id;
        temp_input.addEventListener("click", set_day);
        
        if (0xFF == interval.days)
        {
            temp_input.checked = true;
        }
        else if (0x01 == ((interval.days>>cnt) & (0x01)))
        {
            temp_input.checked = true;
        }

        temp_div.appendChild(temp_input);
        div.appendChild(temp_div);
        cnt++;
    })
    /************************************************************************/

    const element = document.getElementById("intervals");
    element.appendChild(div);
}

function set_start_time(e)
{
    var idx = get_interval_idx(e.srcElement.id);

    if (intervals[idx].stop_time != "")
    {
        var start = String(e.srcElement.value).split(':');
        var stop  = intervals[idx].stop_time.split(':');

        if (start[0] > stop[0])
        {
            alert("Start time cannot be after stop time!");
            e.srcElement.value = '';
        }
        else if (start[0] == stop[0])
        {
            if (start[1] > stop[1])
            {
                alert("Start time cannot be after stop time!");
                e.srcElement.value = '';
            }
            else
            {
            intervals[idx].start_time = e.srcElement.value;
            }
        }
        else
        {
            intervals[idx].start_time = e.srcElement.value;
        }
    }
    else
    {
        intervals[idx].start_time = e.srcElement.value;
    }

    console.log(idx);
    console.log(intervals[idx].start_time);
}

function set_stop_time(e)
{
    var idx = get_interval_idx(e.srcElement.id);

    if (intervals[idx].start_time != "")
    {
        var start = intervals[idx].start_time.split(':');
        var stop  = String(e.srcElement.value).split(':');

        if (start[0] > stop[0])
        {
            alert("Stop time cannot be before start time!");
            e.srcElement.value = '';
        }
        else if (start[0] == stop[0])
        {
            if (start[1] > stop[1])
            {
                alert("Stop time cannot be before start time!");
                e.srcElement.value = '';
            }
            else
            {
            intervals[idx].stop_time = e.srcElement.value;
            }
        }
        else
        {
            intervals[idx].stop_time = e.srcElement.value;
        }
    }
    else
    {
        intervals[idx].stop_time = e.srcElement.value;
    }

    console.log(idx);
    console.log(intervals[idx].stop_time);
}

function set_day(e)
{
    var idx = get_interval_idx(e.srcElement.id);

    switch(e.srcElement.name)
    {
        case "m":
            console.log("Monday");
            if (e.srcElement.checked)
            {
                intervals[idx].days |= 0x01;
            }
            else
            {
                intervals[idx].days &= ~0x01;
            }
            break;
        case 't':
            console.log("Tuesday");
            if (e.srcElement.checked)
            {
                intervals[idx].days |= 0x02;
            }
            else
            {
                intervals[idx].days &= ~0x02;
            }
            break;
        case 'w':
            console.log("Wednesday");
            if (e.srcElement.checked)
            {
                intervals[idx].days |= 0x04;
            }
            else
            {
                intervals[idx].days &= ~0x04;
            }
            break;
        case 'th':
            console.log("Thursday");
            if (e.srcElement.checked)
            {
                intervals[idx].days |= 0x08;
            }
            else
            {
                intervals[idx].days &= ~0x08;
            }
            break;
        case 'f':
            console.log("Friday");
            if (e.srcElement.checked)
            {
                intervals[idx].days |= 0x10;
            }
            else
            {
                intervals[idx].days &= ~0x10;
            }
            break;
        case 'sat':
            console.log("Saturday");
            if (e.srcElement.checked)
            {
                intervals[idx].days |= 0x20;
            }
            else
            {
                intervals[idx].days &= ~0x20;
            }
            break;
        case 'sun':
            console.log("Sunday");
            if (e.srcElement.checked)
            {
                intervals[idx].days |= 0x40;
            }
            else
            {
                intervals[idx].days &= ~0x40;
            }
            break;
        case 'all':
            console.log("All");
            if (e.srcElement.checked)
            {
                intervals[idx].days |= 0xFF;
                days.forEach(day => {
                    element = document.getElementById(day + "_" + idx);
                    element.checked = true;
                })
            }
            else
            {
                intervals[idx].days &= ~0xFF;
                days.forEach(day => {
                    element = document.getElementById(day + "_" + idx);
                    element.checked = false;
                })
            }
            break;
    }

    console.log(intervals[idx].days);
}

function post_intervals()
{
    console.log(JSON.stringify(intervals));

    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open("POST", "ts.cgi");
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.send(JSON.stringify(intervals));

    xmlhttp.onreadystatechange = function()
    {
        if (this.readyState == 4 && this.status == 200)
        {
            console.log(this.responseText);
        }
    };
}

function post_zip()
{
    const element = document.getElementById("zip");

    var isnum = /^\d+$/.test(element.value);

    if (!isnum || element.value.length != 5)
    {
        alert("Invalid ZIP code. Please input 5 digits.");
    }
    else
    {
        var obj = new Object();
        obj.zip = parseInt(element.value);
        console.log(JSON.stringify(obj));

        var xmlhttp = new XMLHttpRequest();
        xmlhttp.open("POST", "zip.cgi");
        xmlhttp.setRequestHeader("Content-Type", "application/json");
        xmlhttp.send(JSON.stringify(obj));

        xmlhttp.onreadystatechange = function()
        {
            if (this.readyState == 4 && this.status == 200)
            {
                console.log(this.responseText);
            }
        };

    }
}

function get_interval_idx(ele_id_str)
{
    var temp_str = String(ele_id_str);
    var start = temp_str.indexOf("_") + 1;
    var end = temp_str.length;

    return(temp_str.substring(start, end));
}
