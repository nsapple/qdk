def can_build(env, platform):
    # VR is not required for this fork, so the mobile VR module stays disabled on all
    # platforms to avoid pulling in unused dependencies or editor integration.
    return False


def configure(env):
    pass


def get_doc_classes():
    return [
        "MobileVRInterface",
    ]


def get_doc_path():
    return "doc_classes"
