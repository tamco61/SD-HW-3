import pathlib
import pytest

pytest_plugins = ['pytest_userver.plugins.core', 'pytest_userver.plugins.postgresql']
from testsuite.databases.pgsql import discover


@pytest.fixture(scope='session')
def service_source_dir():
    """Path to root directory service."""
    return pathlib.Path(__file__).parent.parent

@pytest.fixture(scope='session')
def pgsql_local(service_source_dir, pgsql_local_create):
    databases = discover.find_schemas(
        'blablacar_service',
        [service_source_dir.joinpath('postgresql/schemas')],
    )
    return pgsql_local_create(list(databases.values()))
