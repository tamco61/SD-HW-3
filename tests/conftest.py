import pathlib
import pytest

pytest_plugins = ['pytest_userver.plugins.core']

@pytest.fixture(scope='session')
def service_source_dir():
    """Path to root directory service."""
    return pathlib.Path(__file__).parent.parent
