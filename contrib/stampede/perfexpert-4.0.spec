# See http://www.tacc.utexas.edu/perfexpert/

Summary:    Performance Bottleneck Remediation Tool
Name:       perfexpert
Version:    4.0
Release:    1
License:    LGPLv3
Vendor:     The University of Texas at Austin
Group:      Applications/HPC
Source:     %{name}-%{version}.tar.gz
Packager:   TACC - fialho@tacc.utexas.edu, TACC - carlos@tacc.utexas.edu
Buildroot:  /var/tmp/%{name}-%{version}-buildroot

#------------------------------------------------
# BASIC DEFINITIONS
#------------------------------------------------
# This will define the correct _topdir
%include rpm-dir.inc

%define debug_package %{nil}
#  This is not needed
#  %define _unpackaged_files_terminate_build 0 
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

#------------------------------------------------
# INSTALL SECTION
#------------------------------------------------
%install

# Start with a clean environment
if [ -f "$BASH_ENV" ]
then
        . $BASH_ENV
        export MODULEPATH=/opt/apps/teragrid/modulefiles:/opt/apps/modulefiles:/opt/modulefiles
fi

module load TACC
module unload mvapich2 intel
module load gcc papi hpctoolkit

mkdir -p %{INSTALL_DIR}/etc
mkdir -p %{INSTALL_DIR}/externals
cp -r ./externals/lib %{INSTALL_DIR}/externals
cp -r ./externals/include %{INSTALL_DIR}/externals
cp ./contrib/stampede/*.conf %{INSTALL_DIR}/etc

mkdir -p $RPM_BUILD_ROOT/%{INSTALL_DIR}

# Now install perfexpert itself
./configure --prefix=%{INSTALL_DIR} --with-rose=%{INSTALL_DIR}/externals --with-jvm=/usr/java/latest/jre/lib/amd64/server/ --with-papi=$TACC_PAPI_DIR
make install

# Add all the module stuff here
mkdir -p $RPM_BUILD_ROOT/%{MODULE_DIR}
cat > $RPM_BUILD_ROOT/%{MODULE_DIR}/%{version}.lua << 'EOF'

local help_message = [[
The perfexpert module makes TACC_PERFEXPERT_DIR
available, and adds the PerfExpert directory to your PATH.

WARNING:	
This version of PerfExpert will actually try to change 
your source code. Make sure to have a copy before running it.

For a detailed description of PerfExpert visit:
http://www.tacc.utexas.edu/perfexpert/
	
Version 4.0
]]

whatis("PerfExpert")
whatis("Version: 4.0")
whatis("Category: application, hpc")
whatis("Description: Performance Bottleneck Remediation Tool")
whatis("URL: http://www.tacc.utexas.edu/perfexpert/")

help(help_message,"\n")

if( mode() == "load" ) then
  LmodMessage("WARNING:\nThis version of PerfExpert will actually try to change\nyour source code. Make sure to have a copy before running it.\n")
end

-- Prerequisites
prereq("papi")
prereq("hpctoolkit")

-- Export environmental variables
local pe_dir="%{INSTALL_DIR}"
setenv("TACC_PERFEXPERT_DIR",pe_dir)
setenv("TACC_PERFEXPERT_BIN",pathJoin(pe_dir,"/bin"))

-- Prepend PerfExpert directories to the adequate PATH variables
prepend_path("PATH",pathJoin(pe_dir,"/bin"))
prepend_path("PATH",pathJoin(pe_dir,"/externals/bin"))
prepend_path("LD_LIBRARY_PATH",pathJoin(pe_dir,"/externals/lib"))
prepend_path("LD_LIBRARY_PATH","/usr/java/latest/jre/lib/amd64/server/")

EOF

%{SPEC_DIR}/checkModuleSyntax $RPM_BUILD_ROOT/%{MODULE_DIR}/%{version}.lua

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
