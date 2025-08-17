const js = require('@eslint/js');

module.exports = [
  {
    ignores: [
      '**/build/**',
      'node_modules/**',
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
      'scripts/',
    ],
  },
  js.configs.recommended,
];
