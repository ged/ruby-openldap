#!/usr/bin/env rake

require 'pp'
require 'rbconfig'
require 'pathname'

begin
	require 'rake/extensiontask'
rescue LoadError
	abort "This Rakefile requires rake-compiler (gem install rake-compiler)"
end

begin
	require 'hoe'
rescue LoadError
	abort "This Rakefile requires hoe (gem install hoe)"
end

# Build constants
BASEDIR = Pathname( __FILE__ ).dirname.relative_path_from( Pathname.pwd )
SPECDIR = BASEDIR + 'spec'
LIBDIR  = BASEDIR + 'lib'
EXTDIR  = BASEDIR + 'ext'

DLEXT   = Config::CONFIG['DLEXT']
EXT     = LIBDIR + "bluecloth_ext.#{DLEXT}"

VALGRIND_OPTIONS = [
	"--num-callers=50",
	"--error-limit=no",
	"--partial-loads-ok=yes",
	"--undef-value-errors=no",
]
VALGRIND_MEMORYFILL_OPTIONS = [
	"--freelist-vol=100000000",
	"--malloc-fill=6D",
	"--free-fill=66 ",
]

GDB_OPTIONS = []


# Load Hoe plugins
Hoe.plugin :mercurial
Hoe.plugin :yard
Hoe.plugin :signing

Hoe.plugins.delete :rubyforge
Hoe.plugins.delete :compiler

# Configure Hoe
hoespec = Hoe.spec 'openldap' do
	self.readme_file = 'README.md'
	self.history_file = 'History.md'

	self.developer 'Michael Granger', 'ged@FaerieMUD.org'

	self.dependency 'rake-compiler', '~> 0.7', :developer
	self.dependency 'rspec',         '~> 2.6', :developer

	self.spec_extras[:licenses] = ["BSD"]
	self.spec_extras[:signing_key] = '/Volumes/Keys/ged-private_gem_key.pem'
	self.spec_extras[:extensions] = [ EXTDIR + 'extconf.rb' ]

	self.require_ruby_version( '>= 1.9.2' )

	self.hg_sign_tags = true if self.respond_to?( :hg_sign_tags= )
	self.yard_opts = [ '--protected', '--verbose' ] if self.respond_to?( :yard_opts= )

	self.rdoc_locations << "deveiate:/usr/local/www/public/code/#{remote_rdoc_dir}"
end

ENV['VERSION'] ||= hoespec.spec.version.to_s

# Ensure the specs pass before checking in
task 'hg:precheckin' => :spec

# Ensure the extension is compiled before testing
task :spec => :compile

# gem-testers support
task :test do
	# rake-compiler always wants to copy the compiled extension into lib/, but
	# we don't want testers to have to re-compile, especially since that
	# often fails because they can't (and shouldn't have to) write to tmp/ in
	# the installed gem dir. So we clear the task rake-compiler set up
	# to break the dependency between :spec and :compile when running under
	# rubygems-test, and then run :spec.
	Rake::Task[ EXT.to_s ].clear
	Rake::Task[ :spec ].execute
end

desc "Turn on warnings and debugging in the build."
task :maint do
	ENV['MAINTAINER_MODE'] = 'yes'
end

ENV['RUBY_CC_VERSION'] = '1.8.7:1.9.2'

# Rake-compiler task
Rake::ExtensionTask.new do |ext|
	ext.name           = 'openldap_ext'
	ext.gem_spec       = hoespec.spec
	ext.ext_dir        = 'ext'
	ext.source_pattern = "*.{c,h}"
	ext.cross_compile  = true
	ext.cross_platform = %w[i386-mswin32 i386-mingw32]
end


namespace :spec do

	RSPEC_CMD = [ RUBY, '-S', 'rspec', '-Ilib:ext', SPECDIR.to_s ]

	desc "Run the specs under GDB."
	task :gdb => [ :compile ] do |task|
		cmd = [ 'gdb' ] + GDB_OPTIONS
		cmd += [ '--args' ]
		cmd += RSPEC_CMD
		run( *cmd )
	end

	desc "Run the specs under Valgrind."
	task :valgrind do
		cmd = [ 'valgrind' ] + VALGRIND_OPTIONS
		cmd += RSPEC_CMD
		run( *cmd )
	end

end

### Make the ChangeLog update if the repo has changed since it was last built
file '.hg/branch'
file 'ChangeLog' => '.hg/branch' do |task|
	$stderr.puts "Updating the changelog..."
	content = nil

	begin
		content = make_changelog()
	rescue NoMethodError
		abort "Releasing requires the hoe-mercurial plugin (gem install hoe-mercurial)"
	end

	File.open( task.name, 'w', 0644 ) do |fh|
		fh.print( content )
	end
end

# Rebuild the ChangeLog immediately before release
task :prerelease => 'ChangeLog'

