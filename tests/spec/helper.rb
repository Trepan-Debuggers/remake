unless defined?(SPEC_DIR)
  SPEC_DIR = File.dirname(__FILE__)
  EXAMPLE_DIR = File.join(SPEC_DIR, 'example')
  DATA_DIR = File.join(SPEC_DIR, 'data')
  MAKE_PROG = File.join(SPEC_DIR, '..', '..', 'remake')
end
