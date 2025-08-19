const js = require('@eslint/js');
const globals = require('globals');

module.exports = [
  js.configs.recommended,
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
      globals: { ...globals.node, ...globals.es2021 },
    },
    ignores: ['**/node_modules/**', '**/build/**', 'apps/**', 'assets/**', 'docs/**', 'shaders/**', 'shaders_vk/**', 'tests/**', 'tools/**', 'scripts/**', 'cmake/**'],
    rules: {
      'no-unused-vars': 'warn',
      'semi': ['error', 'always'],
    },
  },
];
