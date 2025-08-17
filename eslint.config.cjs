const js = require('@eslint/js');
const globals = require('globals');

module.exports = [
  {
    ignores: [
      '**/build/**',
      'node_modules/**',
      'docs/**',
      'assets/**',
      'tests/**',
      'src/**',
      'apps/**',
      'scripts/**',
    ],
  },
  js.configs.recommended,
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
      globals: { ...globals.node, ...globals.es2021 },
    },
    rules: { 'no-unused-vars': 'warn', semi: ['error', 'always'] },
  },
];

