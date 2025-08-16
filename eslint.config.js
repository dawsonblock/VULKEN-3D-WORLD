/** @type {import('eslint').Linter.FlatConfig[]} */
module.exports = [
  {
    ignores: ["**/build/**"]
  },
  {
    files: ["**/*.{js,jsx,ts,tsx}"],
    languageOptions: {
      ecmaVersion: 2020,
      sourceType: "module"
    },
    rules: {
      "no-unused-vars": "warn",
      "semi": ["error", "always"]
    }
  }
];
