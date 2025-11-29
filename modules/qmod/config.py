# qmod module build configuration

def can_build(env, platform):
    # Only build the editor-facing qmod exporter when tools are enabled.
    return env["tools"]


def configure(env):
    pass
