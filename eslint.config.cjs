const js = require("@eslint/js");
const globals = require("globals");

module.exports = [
  {
    ignores: [
      "assets/**",
      "build_ci_sanity/**",
      "cmake/**",
      "docs/**",
      "node_modules/**",
      "scripts/**",
      "shaders/**",
      "shaders_vk/**",
      "tests/**",
      "tools/**",
      "src/**",
      "apps/**"
    ]
  },
  js.configs.recommended,
  {
    languageOptions: {
      globals: {
        ...globals.node
      }
    }
  }
];
