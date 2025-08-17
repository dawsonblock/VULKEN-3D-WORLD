const js = require('@eslint/js');

module.exports = [
  js.configs.recommended,
  {
    ignores: ['node_modules/**'],
    rules: {
      semi: ['error', 'always'],
      'no-unused-vars': 'warn'
    }
  }
];
