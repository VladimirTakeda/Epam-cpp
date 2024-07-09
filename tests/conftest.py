import pytest


def pytest_addoption(parser):
    parser.addoption(
        "--binary-path", action="store", required=True, help="Path to the binary"
    )
    parser.addoption(
        "--project-dir", action="store", required=True, help="Path to the project directory"
    )
