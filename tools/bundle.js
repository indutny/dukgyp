#!/usr/bin/env node
'use strict';

const argv = require('yargs')
    .alias('o', 'output')
    .argv;

const fs = require('fs');
const path = require('path');

const known = new Map();

const inputs = argv._.map((input) => {
  const res = {
    name: path.basename(input),
    content: fs.readFileSync(input).toString()
  };
  res.content = `/* ----- ${res.name} ----- */\n` +
                res.content + '\n' +
                `/* ----- ${res.name} end ----- */\n`;

  known.set(res.name, res);
  return res;
});
const output = argv.output;

inputs.forEach((file) => {
  const content = file.content.replace(/#\s*include\s+"([^"]+)"/g,
                                       (all, name) => {
    const base = path.basename(name);
    if (known.has(base)) {
      const res = known.get(base).content.toString();
      // Include just once
      known.set(base, { content: '' });
      return res;
    } else {
      return all;
    }
  });

  file.content = content;
});

const sources = inputs.filter(input => /\.c$/.test(input.name));

fs.writeFileSync(output, sources.map(source => source.content).join('\n'));
