const js = require('@eslint/js');


module.exports = [
  js.configs.recommended,
  {
    ignores: [

module.exports = [js.configs.recommended];

module.exports = [
  js.configs.recommended,
  {

    ignores: ['node_modules/**', 'tests/**', 'src/**', 'docs/**'],
  },

    ignores: [
      '**/build/**',
        main
      'node_modules/**',
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
    ],
  },
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
      globals: { ...globals.node, ...globals.es2021 },
    },
    rules: {
      'no-unused-vars': 'warn',

      semi: ['error', 'always'],
    },
  },
];


      'semi': ['error', 'always']
    }
  }

      semi: ['error', 'always'],
    },
  },
        main
        main
];

        main
        main
