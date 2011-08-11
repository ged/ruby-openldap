# openldap

* http://bitbucket.org/ged/ruby-openldap


## Description

A simple, but feature-complete Ruby binding for OpenLDAP's libldap. 

This binding is intended as an alternative for [ruby-ldap][] for libraries or applications which require a more complete implementation of the LDAP protocol (according to [RFC4511][]) than it provides.

Additions or changes:

* Referrals for add, modify, delete, modrdn, compare
* Controls for add, modify, delete, modrdn, compare
* Asynchronous and synchronous APIs
* Detailed exception class hierarchy for results instead of just one
  class for all non-success results.
* Complete [RFC4511][] support:
  - extended operations and results
  - unsolicited notifications
  - continuation references
  - intermediate responses
  - alias deferencing
  - etc.
* Cleanly abandon terminated operations where supported
* Memory-handling cleanup to avoid leaks, corruption, and other
  problems experienced in the wild.
* Drop deprecated non-_ext variants of operations which have a 
  modern equivalent.
* M17n for Ruby 1.9.x.
* Improved test coverage

**NOTE:** This library is still under development, and should not be considered to be feature-complete or production-ready.

This project's versions follow the [Semantic Versioning Specification][semver].


## Installation

    gem install openldap

You may have to specify the path to `libldap` like so:

    gem install openldap -- --with-ldap-dir=/usr/local

and/or override your Ruby implementation's architecture flags if your Ruby and OpenLDAP installations don't match, particularly under MacOS X:

    ARCHFLAGS="-arch x86_64" gem install openldap


## Contributing

You can check out the current development source with Mercurial via its [Bitbucket project][bitbucket]. Or if you prefer Git, via [its Github mirror][github].

After checking out the source, run:

    $ rake newb

This task will install any missing dependencies, run the tests/specs,
and generate the API documentation.


## License

Copyright (c) 2011, Michael Granger
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the author/s, nor the names of the project's
  contributors may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


[RFC4511]: http://tools.ietf.org/html/rfc4511
[ruby-ldap]: http://ruby-ldap.sourceforge.net/
[semver]: http://semver.org/
[bitbucket]: https://bitbucket.org/ged/ruby-openldap
[github]: https://github.com/ged/ruby-openldap

