module.exports = [
  {
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: 'script',
    },
    ignores: [
      'build/**',
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
      'scripts/**',
      'eslint.config.cjs',
    ],
    rules: {
      'no-unused-vars': 'warn',
      'semi': ['error', 'always'],
    },
  },
];
