const js = require('@eslint/js');
const globals = require('globals');

module.exports = [
  js.configs.recommended,
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'commonjs',
      globals: { ...globals.node, ...globals.es2021 }
    },
    ignores: [
      'build/**',
      'build_ci_sanity/**',
      'cmake/**',
      'docs/**',
      'assets/**',
      'shaders/**',
      'shaders_vk/**',
      'tools/**',
      'tests/**',
      'apps/**',
      'scripts/**',
      'node_modules/**'
    ],
    rules: {
      'no-unused-vars': 'warn',
      semi: ['error', 'always']
    }
  }
];
