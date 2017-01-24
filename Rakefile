#!/usr/bin/env rake

require 'pp'
require 'rbconfig'
require 'pathname'
require 'rake/clean'

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
BASEDIR      = Pathname( __FILE__ ).dirname.relative_path_from( Pathname.pwd )
SPECDIR      = BASEDIR + 'spec'
LIBDIR       = BASEDIR + 'lib'
EXTDIR       = BASEDIR + 'ext'

TEST_WORKDIR = BASEDIR + 'test_workdir'
CLOBBER.include( TEST_WORKDIR.to_s )

DLEXT        = RbConfig::CONFIG['DLEXT']
EXT          = LIBDIR + "openldap_ext.#{DLEXT}"
RUBY         = RbConfig.expand( "$(bindir)/$(ruby_install_name)" )

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

if Rake.application.trace
	ENV['RUBYOPT'] = 'w'
end


# Load Hoe plugins
Hoe.plugin :mercurial
Hoe.plugin :signing

Hoe.plugins.delete :rubyforge
Hoe.plugins.delete :compiler

# Configure Hoe
hoespec = Hoe.spec 'openldap' do
	self.readme_file = 'README.md'
	self.history_file = 'History.md'
	self.extra_rdoc_files = FileList[ '*.md' ]
	self.urls = {
		home:   'http://deveiate.org/projects/ruby-openldap',
		code:   'http://bitbucket.org/ged/ruby-openldap',
		docs:   'http://deveiate.org/code/ruby-openldap',
		github: 'http://github.com/ged/ruby-openldap',
	}

	self.developer 'Michael Granger', 'ged@FaerieMUD.org'

	self.dependency 'loggability', '~> 0.12'

	self.dependency 'rake-compiler', '~> 0.8', :developer
	self.dependency 'hoe-deveiate',  '~> 0.2', :developer

	self.require_ruby_version( '>= 2.3.3' )

	self.hg_sign_tags = true if self.respond_to?( :hg_sign_tags= )
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

# Rake-compiler task
Rake::ExtensionTask.new( 'openldap_ext' )


namespace :spec do

	RSPEC_CMD = [ RUBY, '-S', 'rspec', SPECDIR.to_s ]
	$stderr.puts "Rspec command: %p" % [ RSPEC_CMD ]

	desc "Run the specs under GDB."
	task :gdb => [ :compile ] do |task|
		cmd = [ 'gdb', '-ex', 'run' ] + GDB_OPTIONS
		cmd += [ '--args' ]
		cmd += RSPEC_CMD
		system( *cmd )
	end

	desc "Run the specs under Valgrind."
	task :valgrind do
		cmd = [ 'valgrind' ] + VALGRIND_OPTIONS
		cmd += RSPEC_CMD
		system( *cmd )
	end

end

# Rebuild the ChangeLog immediately before release
task :prerelease => 'ChangeLog'

CLEAN.include( 'test_workdir', 'tmp' )

