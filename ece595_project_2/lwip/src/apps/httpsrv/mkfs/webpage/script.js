const max_intervals = 5;
var days = ["m", "t", "w", "th", "f", "sat", "sun", "all"];
var intervals = [];
var id_cntr = 0;

var rtc_value = new Array(3);
var data_received = 0;

window.onload = loop;

function parse_vars(data)
{
   const element = document.getElementById("rtc_time");
   element.innerHTML = data;
}

function loop()
{
   if (!data_received)
   {
      var xmlhttp = new XMLHttpRequest();
      xmlhttp.onreadystatechange = function() { alertContents(xmlhttp); };
      xmlhttp.open('GET', "rtcdata.cgi", true);
      xmlhttp.send(null);
   }
   
   setTimeout("loop()", 1000);
}

function alertContents(http_request)
{
   if (http_request.readyState == 4)
    {
      if (http_request.status == 200)
        {
            parse_vars(http_request.responseText);
        }
        data_received = 0;
    }
}

function add_time_interval()
{
   if (num_valid_intervals() < max_intervals)
   {
      const div = document.createElement("div");
      div.className = "row";
      div.id = "row_" + id_cntr;

      /************************************************************************/
      var temp_div = document.createElement("div");
      temp_div.className = "time";

      var temp_input = document.createElement("input");
      temp_input.className = "time_input";
      temp_input.type = "time";
      temp_input.id = "start_" + id_cntr;
      temp_input.addEventListener("input", set_start_time);
      temp_div.appendChild(temp_input);
      div.appendChild(temp_div);
      /************************************************************************/

      /************************************************************************/
      var temp_div = document.createElement("div");
      temp_div.className = "time";

      var temp_input = document.createElement("input");
      temp_input.className = "time_input";
      temp_input.type = "time";
      temp_input.id = "stop_" + id_cntr;
      temp_input.addEventListener("input", set_stop_time);
      temp_div.appendChild(temp_input);
      div.appendChild(temp_div);
      /************************************************************************/

      /************************************************************************/
      days.forEach(day => {
         var temp_div = document.createElement("div");
         temp_div.className = "day";
         temp_div.name = day;

         var temp_input = document.createElement("input");
         temp_input.className = "day_select";
         temp_input.type = "checkbox";
         temp_input.name = day;
         temp_input.id = day + '_' + id_cntr;
         temp_input.addEventListener("click", set_day);
         temp_div.appendChild(temp_input);
         div.appendChild(temp_div);
      })
      /************************************************************************/

      /************************************************************************/
      var temp_div = document.createElement("div");
      temp_div.className = "day";

      var temp_button = document.createElement("button");
      temp_button.innerHTML = "Delete";
      temp_button.id = "delete_" + id_cntr;
      temp_button.className = "delete_button"
      temp_button.addEventListener("click", invalidate_interval);
      temp_div.appendChild(temp_button);
      div.appendChild(temp_div);
      /************************************************************************/

      const element = document.getElementById("intervals");
      element.appendChild(div);

      var interval = new Object();
      interval.id = id_cntr;
      interval.invalidate = false;
      interval.days = 0;
      interval.start_time = "";
      interval.stop_time = "";
      intervals.push(interval);
      id_cntr += 1;
   }
}

function num_valid_intervals()
{
   var i = 0;
   intervals.forEach(interval => {
     if (!interval.invalidate)
     {
        i++;
     }
   })
   
   return i;
}

function invalidate_interval(e)
{
   var idx = get_interval_idx(e.srcElement.id);

   intervals[idx].invalidate = true;
   const element = document.getElementById("row_" + idx);
   element.parentNode.removeChild(element);
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
   var valid_intervals = [];

   intervals.forEach(interval => {
      if (!interval.invalidate)
      {
         var temp = new Object();
         
         temp.id = interval.id;
         temp.days = interval.days;
         temp.start_time = interval.start_time;
         temp.stop_time = interval.stop_time;
         
         valid_intervals.push(temp);
      }
   })
   
   // Always send 5 intervals
   for (var i=valid_intervals.length; i<max_intervals; i++)
   {
      var temp = new Object();
      
      temp.id = -1;
      temp.days = -1;
      temp.start_time = "";
      temp.stop_time = "";
      
      valid_intervals.push(temp);
   }
   
   console.log(JSON.stringify(valid_intervals));

   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", "post.cgi");
   xmlhttp.setRequestHeader("Content-Type", "application/json");
   xmlhttp.send(JSON.stringify(valid_intervals));

   xmlhttp.onreadystatechange = function()
   {
      if (this.readyState == 4 && this.status == 200)
      {
         console.log(this.responseText);
      }
   };
}

function get_interval_idx(ele_id_str)
{
   var temp_str = String(ele_id_str);
   var start = temp_str.indexOf("_") + 1;
   var end = temp_str.length;

   return(temp_str.substring(start, end));
}
