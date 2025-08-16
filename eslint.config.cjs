const js = require('@eslint/js');

module.exports = [
  {
    ignores: [
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
      'node_modules/**'
    ]
  },
  js.configs.recommended
];
