import sys
import glob

# Point Python directly to the physically extracted gem on the hard drive
for p in glob.glob('/gems/gems/openc3-cosmos-nos3-*/targets/SAMPLE/scripts'):
    if p not in sys.path:
        sys.path.append(p)

from openc3.script.suite import Suite, Group

try:
    from nos3.sample_lib import safe_sample
    from nos3.sample_app_test import run_sample_app_test
    from nos3.sample_device_test import run_sample_device_test
    from nos3.sample_ast_test import run_sample_ast_test
except ImportError:
    pass

class SAMPLE_Functional_Test(Group):
    def setup(self):
        safe_sample()

    def script_application(self):
        run_sample_app_test()

    def script_device(self):
        run_sample_device_test()

    def teardown(self):
        safe_sample()

class SAMPLE_Automated_Scenario_Test(Group):
    def setup(self):
        safe_sample()

    def script_ast(self):
        run_sample_ast_test()

    def teardown(self):
        safe_sample()

class Sample_Test(Suite):
    def __init__(self):
        super().__init__()
        self.add_group(SAMPLE_Functional_Test)
        self.add_group(SAMPLE_Automated_Scenario_Test)

    def setup(self):
        safe_sample()
        
    def teardown(self):
        safe_sample()