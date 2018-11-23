const streamlabs_overlays = require('../build/Release/streamlabs_overlay');

console.log('Call start');
streamlabs_overlays.start();

function script_finished() {	
	console.log('--------------- script finished');
	console.log('');
	console.log('Call stop');
	streamlabs_overlays.stop();

    console.log('Script end');
}

function finish_program() 
{
	console.log('--------------- finish program');
	console.log('');
	//console.log('Call get count');
	//console.log(streamlabs_overlays.get_overlays_count());

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.get_overlays_ids();
	console.log(overlays_ids);

	console.log('Call get overlay info ' + overlays_ids[0]);
	console.log(streamlabs_overlays.get_overlay_info(overlays_ids[0]));

	console.log('Call remove overlay');
	streamlabs_overlays.remove_overlay(overlays_ids[0]);

	//console.log('Call hide');
	//streamlabs_overlays.hide_overlays();
	
	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.get_overlays_ids();
	console.log(overlays_ids);

	//console.log('Call get overlay info ' + overlays_ids[0]);
	//console.log(streamlabs_overlays.get_overlay_info(overlays_ids[0]));
	
	setTimeout(script_finished, 2000);
}

function start_test() 
{
	console.log('--------------- start_test');
	console.log('');
	console.log('Call get count');
	console.log(streamlabs_overlays.get_overlays_count());

	console.log('Call add overlay');
	console.log(streamlabs_overlays.add_overlay("https://www.google.com/doodles/"));

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.get_overlays_ids();
	console.log(overlays_ids);

	//console.log('Call get overlay info ' + overlays_ids[0]);
	//console.log(streamlabs_overlays.get_overlay_info(overlays_ids[0]));

	//console.log('Call add overlay with size');
	//console.log(streamlabs_overlays.add_overlay_ex("https://www.google.com/", 200, 50, 400, 200));
	
	//console.log('Call set overlay position');
	//console.log(streamlabs_overlays.set_overlay_position(overlays_ids[0], 2000, 50, 400, 200));

	console.log('Call set overlay url');
	console.log(streamlabs_overlays.set_overlay_url(overlays_ids[0], "http://bing.com/"));
	
	console.log('Call show');
	streamlabs_overlays.show_overlays();
	
	setTimeout(finish_program, 7000);
}

function light_test() 
{
	console.log('--------------- light_test');
	console.log('');
	console.log('Call get count');
	console.log(streamlabs_overlays.get_overlays_count());

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.get_overlays_ids();
	console.log(overlays_ids);

	console.log('Call get overlay info ' + overlays_ids[0]);
	console.log(streamlabs_overlays.get_overlay_info(overlays_ids[0]));

	console.log('Call add overlay');
	console.log(streamlabs_overlays.add_overlay("http://google.com"));

	console.log('Call get count');
	//console.log(streamlabs_overlays.get_overlays_count());

	console.log('Call show');
	//streamlabs_overlays.show_overlays();
	
	setTimeout(script_finished, 20000);
}

setTimeout(start_test, 2000);
//setTimeout(light_test, 2000);

setTimeout(script_finished, 12000);