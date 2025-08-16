/** @type {import('eslint').Linter.FlatConfig[]} */
export default [
  { ignores: ['**/build/**', 'eslint.config.cjs'] },
  {
    files: ['**/*.{js,jsx,ts,tsx}'],
    languageOptions: { ecmaVersion: 2020, sourceType: 'module' },
    rules: { 'no-unused-vars': 'warn', 'semi': ['error', 'always'] }
  }
];
