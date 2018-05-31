obtain(['µ/serialParser.js', 'events'], ({ serialParser }, EventEmitter)=> {
  const COLUMN_COLOR = 64;
  const READY = 127;
  const BALL_DETECT = 32;
  const NUM_COLUMNS = 16;

  class BallWall extends EventEmitter{
    constructor(conf) {
      super();
      var _this = this;
      var parser = new serialParser();

      _this.columns = [];

      parser.on(BALL_DETECT, (data)=> {
        if (_this.columns.length) _this.columns[data[0]].detect();
      });

      parser.on(NUM_COLUMNS, (data)=> {
        //_this.columns.length = data[0];
        for (let i = 0; i < data[0]; i++) {
          _this.columns.push({
            detectTO: null,
            justRead: false,
            detect() {
              var _that = this;

              if (!_that.justRead) {
                _that.justRead = true;
                _this.emit('ballDetect', i);
                _that.detectTO = setTimeout(()=> {_that.justRead = false;}, 200);
              }
            },
          });
        }

        _this.emit('enumerate', data[0]);
      });

      _this.setColumnColor = (column, color)=> {
        if (column <= _this.columns.length) {
          parser.sendPacket([1, COLUMN_COLOR, column, color[0], color[1], color[2]]);
        }
      };

      var readyInt;

      parser.onOpen = ()=> {
        parser.sendPacket([1, READY]);
      };

      parser.on(READY, ()=> {
        if (!_this.ready) {
          console.log('Arduino ready');
          clearInterval(readyInt);
          _this.ready = true;
          _this.emit('ready');
        }
      });

      _this.whenReady = (cb)=> {
        if (_this.ready) {
          cb();
        } else {
          this.on('ready', cb);
        }
      };

      if (conf.name) parser.setup({ name: conf.name, baud: 115200 });
      else if (conf.manufacturer) parser.setup({ manufacturer: conf.manufacturer, baud: 115200 });

    }

    set onready(cb) {
      //this.on_load = val;
      if (this.ready) {
        cb();
      } else {
        this.on('ready', cb);
      }
    }

  };

  exports.BallWall = BallWall;
});
