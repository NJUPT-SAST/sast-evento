"""Group logs"""

import os
import sys


class Grouping:
    """Group logs"""

    def __init__(self, name):
        self.name = name
        self._gha = "GITHUB_ACTIONS" in os.environ

    def __enter__(self):
        if self._gha:
            print(f"::group::{self.name}")
        else:
            print(f"==== {self.name} ====")
        sys.stdout.flush()
        sys.stderr.flush()

    def __exit__(self, exc_type, exc_value, traceback):
        if self._gha:
            print("::endgroup::")
        sys.stdout.flush()
        sys.stderr.flush()
