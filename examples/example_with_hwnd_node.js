const streamlabs_overlays = require('../index')

console.log('Call start');
streamlabs_overlays.start();

function script_finished() {
	console.log('--------------- script_finished');
	console.log('');

	console.log('Call stop');
	streamlabs_overlays.stop();

    console.log('END');
}

function step_finish()
{
	console.log('--------------- step_finish');
	console.log('');


	setTimeout(script_finished, 2000);
}

function step_3()
{
	console.log('--------------- step_3');
	console.log('');

	setTimeout(step_finish, 5000);
}

function step_2()
{
	console.log('--------------- step_2');
	console.log('');

	setTimeout(step_3, 5000);
}

function step_1()
{
	console.log('Call show');
	streamlabs_overlays.show();

    console.log('--------------- step_1');
	console.log('');
	console.log('Call get count');
	console.log(streamlabs_overlays.getCount());

	console.log('Call add overlay');
	console.log(streamlabs_overlays.addHWND( new Buffer("asdf") ) );

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.getIds();
	console.log(overlays_ids);

	console.log('Call get overlay info ' + overlays_ids[0]);
	console.log(streamlabs_overlays.getInfo(overlays_ids[0]));

	console.log('Call show');
	streamlabs_overlays.show();

	setTimeout(step_2, 5000);
}

setTimeout(step_1, 2000);

setTimeout(script_finished, 25000);