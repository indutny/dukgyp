'use strict';

const path = require('path');

const LIB = path.join(__dirname, 'lib');
const SRC = path.join(__dirname, 'src');

const loaders = [
  {
    test: /\.js$/,
    loader: 'babel-loader',
    query: {
      presets: [ 'es2015' ]
    }
  }
];

module.exports = [{
  target: 'web',
  entry: path.join(SRC, 'dukgyp.js'),
  output: {
    path: LIB,
    filename: 'dukgyp.js'
  },
  module: {
    loaders: loaders
  }
}, {
  target: 'web',
  entry: path.join(SRC, 'test.js'),
  output: {
    path: LIB,
    filename: 'test.js'
  },
  module: {
    loaders: loaders
  }
}];
