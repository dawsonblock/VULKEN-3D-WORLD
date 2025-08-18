
/* global require, module */




        main
const js = require('@eslint/js');

        main
        main
module.exports = [

  js.configs.recommended,
  {

    ignores: ['**/*'],
  },

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
        main
  {
    ignores: ['node_modules/**'],
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
    },

    rules: {},
  },
  js.configs.recommended,

    rules: {
      'no-unused-vars': 'warn',
      'semi': ['error', 'always']
    }
        main
  }
        main
        main
];
