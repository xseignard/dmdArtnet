const artnet = require('artnet');

const connection = artnet({
	host: '192.168.1.2',
});

let c = 0;
setInterval(() => {
	console.log('sending ' + c + ' to all channels in all universes');
	for (var i = 0; i < 8; i++) {
		(function(_i, _c) {
			connection.set(
				_i,
				1,
				Array.apply(null, new Array(512)).map(function() {
					return _c;
				}, 0),
				function(err, res) {
					if (err) console.error('error sending to universe ' + i, err);
					else if (res === 530)
						console.log('sent ' + _c + ' to all channels in universe ' + _i);
					else
						console.error(
							'error sending to universe ' +
								_i +
								': only ' +
								res +
								' bytes where sent'
						);
				}
			);
		})(i, c);
	}
	c = c === 1 ? 0 : 1;
}, 100);
