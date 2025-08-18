const js = require('@eslint/js');

module.exports = [
  js.configs.recommended,
  {
    files: ['**/*.json'],
    languageOptions: { parser: require('jsonc-eslint-parser') },
  },
];
