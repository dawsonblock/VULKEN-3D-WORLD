/* eslint-env node */
const js = require('@eslint/js');

module.exports = [
  js.configs.recommended,
  {
    ignores: ['**/build/**', 'node_modules/**', 'eslint.config.cjs'],
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
    },
    rules: {
      'no-unused-vars': 'warn',
      semi: ['error', 'always'],
    },
  },
];
