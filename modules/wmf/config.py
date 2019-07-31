def can_build(env, platform):
    return platform not in ['iphone', 'linux', 'x11', 'mac']

def configure(env):
    pass

def get_doc_classes():
    return []

def get_doc_path():
    return ""
