require 'cosmos'
require 'cosmos/script'
require 'mission_lib.rb'

class LPT < Cosmos::Test
  def setup

  end

  def test_lpt
      start("tests/sample_lpt_test.rb")
  end

  def teardown
    
  end
end

class CPT < Cosmos::Test
  def setup
      
  end

  def test_cpt
      start("tests/sample_cpt_test.rb")
  end

  def teardown

  end
end

class Sample_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('CPT')
      add_test('LPT')
  end

  def setup
  end
  
  def teardown
  end
end