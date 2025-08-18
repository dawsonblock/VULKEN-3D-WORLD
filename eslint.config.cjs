/* global require, module */
const js = require('@eslint/js');

module.exports = [
  {
    ignores: ['node_modules/**'],
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
    },
    rules: {},
  },
  js.configs.recommended,
];
