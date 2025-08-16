const js = require('@eslint/js');
const globals = require('globals');

module.exports = [
  {
    ignores: [
      'apps',
      'assets',
      'build_ci_sanity',
      'cmake',
      'docs',
      'node_modules',
      'scripts',
      'shaders',
      'shaders_vk',
      'tests',
      'tools',
    ],
  },
  js.configs.recommended,
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
      globals: {
        ...globals.node,
        ...globals.es2021,
      },
    },
    rules: {},
  },
];
