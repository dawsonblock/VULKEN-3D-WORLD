const js = require('@eslint/js');

module.exports = [
  js.configs.recommended,
  {
    ignores: ['**/node_modules/**', '**/build/**', '**/build_ci_sanity/**'],
  },
];

