const js = require('@eslint/js');
const globals = require('globals');










module.exports = [{
  ...js.configs.recommended,
  languageOptions: {
    ecmaVersion: 2021,
    sourceType: 'script',
    globals: { ...globals.node, ...globals.es2021 }
  },
  ignores: [
    'build/**',
    'node_modules/**',
    'cmake/**',
    'docs/**',
    'assets/**',
    'shaders/**',
    'shaders_vk/**',
    'tools/**',
    'tests/**',
    'apps/**',
    'scripts/**'
  ],
  rules: { 'no-unused-vars': 'warn', 'semi': ['error', 'always'] }
}];
        main
        main
        main
        main

module.exports = [
  js.configs.recommended,
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',

      globals: { ...globals.node, ...globals.es2021 },


      globals: {
        ...globals.node,
        ...globals.es2021,
      },

      globals: { ...globals.node, ...globals.es2021 },
        main
        main
    },
    ignores: [
      'build/**',
      'node_modules/**',
      'cmake/**',
      'docs/**',
      'assets/**',
      'shaders/**',
      'shaders_vk/**',
      'tools/**',
      'tests/**',
      'src/**',
      'apps/**',
      'scripts/**',
    ],

  },
];


    rules: {
      'no-unused-vars': 'warn',
      'semi': 'error',
    },
  },
];

  },
];







        main
        main
        main
        main
        main
        main
