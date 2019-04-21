var days = ["m", "t", "w", "th", "f", "sat", "sun", "all"];
var intervals = [];
var id_cntr = 0;

function add_time_interval()
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
     intervals[idx].start_time = e.srcElement.value;

     console.log(idx);
     console.log(intervals[idx].start_time);
 }

 function set_stop_time(e)
 {
     var idx = get_interval_idx(e.srcElement.id);
     intervals[idx].stop_time = e.srcElement.value;

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
 }

 function get_interval_idx(ele_id_str)
 {
     var temp_str = String(ele_id_str);
     var start = temp_str.indexOf("_")+1;
     var end = temp_str.length;

     return(temp_str.substring(start, end));
 }
