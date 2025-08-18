

const js = require('@eslint/js');
const globals = require('globals');

        main
module.exports = [
  js.configs.recommended,
  {
    ignores: [

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

      '**/build/**',
      'node_modules/**',
      'build_ci_sanity/**',
      'cmake/**',
      'docs/**',
      'assets/**',
      'shaders/**',
      'shaders_vk/**',
      'tools/**',
      'tests/**',
      'src/**',
      'apps/**',
      'scripts/**'
    ]
  },
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
      globals: { ...globals.node, ...globals.es2021 }
    },
    rules: {
      'no-unused-vars': 'warn',
      'semi': ['error', 'always']
    }
        main
  }
];
