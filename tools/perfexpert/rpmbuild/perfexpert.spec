# perfexpert.spec :: ashay.rane@tacc.utexas.edu
# See http://www.tacc.utexas.edu/perfexpert/

Summary:    Performance Bottleneck Remediation Tool
Name:       perfexpert
Version:    3.1.0
Release:    3
License:    LGPLv3
Vendor:     The University of Texas at Austin
Group:      Applications/HPC
Source:     %{name}-%{version}.tar.gz
Packager:   TACC - fialho@utexas.edu, ashay.rane@tacc.utexas.edu, carlos@tacc.utexas.edu

# This is the actual installation directory - Careful
BuildRoot:  /var/tmp/%{name}-%{version}-buildroot

#------------------------------------------------
# BASIC DEFINITIONS
#------------------------------------------------
# This will define the correct _topdir
%include rpm-dir.inc

%define system	linux
%define APPS    /opt/apps
%define MODULES modulefiles

Summary: Performance Bottleneck Remediation Tool
Group:   Applications/HPC

#------------------------------------------------
# PACKAGE DESCRIPTION
#------------------------------------------------
%description
%description -n %{name}
PerfExpert is a performance analysis tool that provides concise performance assessment and suggests steps that can be taken by the developer to improve performance.

#------------------------------------------------
# INSTALLATION DIRECTORY
#------------------------------------------------
# Buildroot: defaults to null if not included here
%define INSTALL_DIR %{APPS}/%{name}/%{version}
%define MODULE_DIR  %{APPS}/%{MODULES}/%{name}

# Local variables used for installation
%define HPCTOOLKIT_INSTALL_DIR		%{INSTALL_DIR}/extras/hpctoolkit
%define PERFEXPERT_HPCTOOLKIT_HOME	%{INSTALL_DIR}/extras/hpctoolkit

#------------------------------------------------
# PREPARATION SECTION
#------------------------------------------------
# Use -n <name> if source file different from <name>-<version>.tar.gz
%prep

# Remove older attempts
rm   -rf $RPM_BUILD_ROOT/%{INSTALL_DIR}
mkdir -p $RPM_BUILD_ROOT/%{INSTALL_DIR}

# Unpack source
# This will unpack the source to /tmp/BUILD/name-version
%setup -n %{name}-%{version}

#------------------------------------------------
# BUILD SECTION
#------------------------------------------------
%build

# Start with a clean environment
if [ -f "$BASH_ENV" ]
then
	. $BASH_ENV
	export MODULEPATH=/opt/apps/teragrid/modulefiles:/opt/apps/modulefiles:/opt/modulefiles
fi

module load TACC
module unload CTSSV4 mvapich pgi intel
module load gcc papi

# HPCTOOLKIT MUST BE BUILT FOR PERFEXPERT
cd ./extras/hpctoolkit-externals
./configure
make all
make distclean

cd ../hpctoolkit
./configure --prefix=%{HPCTOOLKIT_INSTALL_DIR} --with-externals=../hpctoolkit-externals --with-papi=$TACC_PAPI_DIR
make

#------------------------------------------------
# INSTALL SECTION
#------------------------------------------------
%install

# Start with a clean environment
if [ -f "$BASH_ENV" ]; then
	. $BASH_ENV
	export MODULEPATH=/opt/apps/teragrid/modulefiles:/opt/apps/modulefiles:/opt/modulefiles
	export PERFEXPERT_HOME=$RPM_BUILD_ROOT/%{INSTALL_DIR}
	export PERFEXPERT_HPCTOOLKIT_HOME=%{INSTALL_DIR}/extras/hpctoolkit
else
	setenv PERFEXPERT_HOME $RPM_BUILD_ROOT/%{INSTALL_DIR}
	setenv PERFEXPERT_HPCTOOLKIT_HOME %{INSTALL_DIR}/extras/hpctoolkit
fi

module load TACC
module unload CTSSV4 mvapich pgi intel
module load gcc papi

mkdir -p $RPM_BUILD_ROOT/%{INSTALL_DIR}

# Install the hpctoolkit build and get back to the original directory
cd ./extras/hpctoolkit
make DESTDIR=$RPM_BUILD_ROOT install
cd ../../

# Now install perfexpert itself
make install

# Add all the module stuff here
mkdir -p $RPM_BUILD_ROOT/%{MODULE_DIR}
cat > $RPM_BUILD_ROOT/%{MODULE_DIR}/%{version} << 'EOF'
#%Module1.0####################################################################
##
## PerfExpert
##
## PerfExpert
proc ModulesHelp { } {
	puts stderr "\n"
	puts stderr "\tThe perfexpert module makes TACC_PERFEXPERT_DIR"
	puts stderr "\tavailable, and adds the PerfExpert directory to your PATH."
	puts stderr "\n"
	puts stderr "For a detailed description of PerfExpert visit:"
	puts stderr "http://www.tacc.utexas.edu/perfexpert/"
	puts stderr "\n"
	puts stderr "\tVersion %{version}\n"
}

module-whatis "PerfExpert"
module-whatis "Version: %{version}"
module-whatis "Category: application, hpc"
module-whatis "Description: Performance Bottleneck Remediation Tool"
module-whatis "URL: http://www.tacc.utexas.edu/perfexpert/"

# Prerequisites
prereq papi java

# Tcl script only
set version %{version}

# Export environmental variables
setenv TACC_PERFEXPERT_DIR %{INSTALL_DIR}
setenv TACC_PERFEXPERT_BIN %{INSTALL_DIR}
setenv PERFEXPERT_HPCTOOLKIT_HOME %{PERFEXPERT_HPCTOOLKIT_HOME}

# Prepend the scalasca directories to the adequate PATH variables
prepend-path PATH %{INSTALL_DIR}
EOF

cat > $RPM_BUILD_ROOT/%{MODULE_DIR}/.version.%{version} << 'EOF'
#%Module1.0####################################################################
## Version file for perfexpert version %{version}
set ModulesVersion "%version"
EOF

#------------------------------------------------
# FILES SECTION
#------------------------------------------------
%files -n %{name}

# Define files permisions, user and group
%defattr(-,root,install)
%{INSTALL_DIR}
%{MODULE_DIR}

#------------------------------------------------
# CLEAN UP SECTION
#------------------------------------------------
%post
%clean
# Make sure we are not within one of the directories we try to delete
cd /tmp

# Remove the source files from /tmp/BUILD
rm -rf /tmp/BUILD/%{name}-%{version}

# Remove the installation files now that the RPM has been generated
rm -rf /var/tmp/%{name}-%{version}-buildroot
