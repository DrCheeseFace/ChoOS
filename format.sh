find -iname "*.c" -o -iname "*.h" | xargs clang-format -i --verbose
git ls-files | xargs  spacers
