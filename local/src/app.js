'use strict';

var remote = require('electron').remote;

var process = remote.process;

//remote.getCurrentWindow().closeDevTools();

var obtains = [
  './src/BallWall.js',
  'µ/color.js',
];

obtain(obtains, ({ BallWall }, { rainbow, Color })=> {

  var hardware = new BallWall({ name: 'usb' });

  exports.app = {};

  var sent = false;

  var data = [];

  var fadeInt;
  var dir = 1;
  var val = 0;

  hardware.on('enumerate', num=> {
    console.log(num);
    for (let i = 0; i < num; i++) {
      setTimeout(()=> {
        hardware.setColumnColor(i + 1, Color([0, 0, 0]));
      }, 20 * i);
    }
  });

  hardware.whenReady(()=> {

  });

  var keyColors = [
    ['1', '2', '3', '4'],
    ['q', 'w', 'e', 'r'],
    ['a', 's', 'd', 'f'],
    ['z', 'x', 'c', 'v'],
  ];

  var colors = [
    Color([0, 0, 64]),
    Color([0, 100, 100]),
    Color([0, 127, 0]),
    Color([127, 127, 127]),
  ];

  var picture = [
    [1, 2, 3, 4],
    [2, 3, 4, 1],
    [3, 4, 1, 2],
    [4, 1, 2, 3],
  ];

  var counts = [0, 0, 0, 0];

  hardware.on('ballDetect', (which)=> {
    console.log(which);
    if (counts[which] < picture.length) {
      let col = picture.map(row => row[which]);
      console.log(col);
      hardware.setColumnColor(which + 1, colors[col[counts[which]] - 1]);
      counts[which]++;
    } else hardware.setColumnColor(which + 1, [0, 0, 0]);

  });

  exports.app.start = ()=> {
    console.log('started');

    document.onkeypress = (e)=> {
      keyColors.forEach((set, i)=> {
        set.forEach((key, j)=> {
          if (key == e.key) hardware.setColumnColor(i + 1, colors[j]);
        });
      });

      if (e.key == ' ') {
        counts.forEach(cnt=>cnt = 0);
      }
    };

    document.onkeyup = (e)=> {
      if (e.which == 27) {
        var electron = require('electron');
        process.kill(process.pid, 'SIGINT');
      } else if (e.which == 73 && e.getModifierState('Control') &&  e.getModifierState('Shift')) {
        remote.getCurrentWindow().toggleDevTools();
      }
    };

    process.on('SIGINT', ()=> {
      process.nextTick(function () { process.exit(0); });
    });
  };

  provide(exports);
});
