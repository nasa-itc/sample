require 'cosmos'
require 'cosmos/script'
require 'mission_lib.rb'

class SAMPLE_LPT < Cosmos::Test
  def setup

  end

  def test_lpt
      start("tests/sample_lpt_test.rb")
  end

  def teardown

  end
end

class SAMPLE_CPT < Cosmos::Test
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
      add_test('SAMPLE_CPT')
      add_test('SAMPLE_LPT')
  end

  def setup
  end
  
  def teardown
  end
end