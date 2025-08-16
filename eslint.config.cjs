const js = require('@eslint/js');
const globals = require('globals');

module.exports = [
  {
    ignores: [

      'assets/**', 'build_ci_sanity/**', 'cmake/**',
      'docs/**', 'shaders/**', 'shaders_vk/**',
      'tools/**'
    ]

      'assets/**',
      'build_ci_sanity/**',
      'cmake/**',
      'docs/**',
      'shaders/**',
      'shaders_vk/**',
      'tools/**',
      'tests/**',
      'src/**',
      'apps/**',
      'scripts/**',
    ],
        main
  },
  js.configs.recommended,
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',

      globals: { ...globals.node, ...globals.es2021 }

      globals: {
        ...globals.node,
        ...globals.es2021,
      },
        main
    },
    rules: {},
  },
];

