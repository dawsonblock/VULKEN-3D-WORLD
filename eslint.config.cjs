const js = require('@eslint/js');
const globals = require('globals');

module.exports = [
  js.configs.recommended,
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
      globals: { ...globals.node, ...globals.es2021 }
    },
    ignores: [
      'build/**',
      'build_ci_sanity/**',
      'node_modules/**'
    ],
    rules: {
      'no-unused-vars': 'warn',
      'semi': ['error', 'always']
    }
  }
];
