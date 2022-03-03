std = "luajit"
globals = {
    "box",
    "tonumber64",
}
ignore = {
    -- Accessing an undefined field of a global variable <package>.
    "143/package",
    -- Unused argument <self>.
    "212/self",
    -- Redefining a local variable.
    "411",
    -- Redefining an argument.
    "412",
    -- Shadowing a local variable.
    "421",
    -- Shadowing an upvalue.
    "431",
    -- Shadowing an upvalue argument.
    "432",
}

include_files = {
    "**/*.lua",
}

exclude_files = {
    "build/**/*.lua",
    "test-run/**/*.lua",
    ".rocks/**/*.lua",
    ".git/**/*.lua",
}
