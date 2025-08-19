const globals = require('globals');

module.exports = [
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
      globals: { ...globals.node, ...globals.es2021 }
    },
    ignores: [
      'build/**',
      'node_modules/',
      'build_ci_sanity/',
      'cmake/',
      'docs/',
      'assets/',
      'shaders/',
      'shaders_vk/',
      'tools/',
      'tests/',
      'src/',
      'apps/',
      'scripts/'
    ]
  }
];
