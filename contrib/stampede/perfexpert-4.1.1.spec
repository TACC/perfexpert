# See http://www.tacc.utexas.edu/perfexpert/ -- project page
#     https://github.com/TACC/perfexpert/ -- source code
#     http://tacc.github.io/perfexpert/ -- online documenation

Summary:    Performance Bottleneck Remediation Tool
Name:       perfexpert
Version:    4.1.1
Release:    1
License:    UTRL
Vendor:     The University of Texas at Austin
Group:      Applications/HPC
Source:     %{name}-%{version}.tar.gz
Packager:   TACC - fialho@tacc.utexas.edu, TACC - carlos@tacc.utexas.edu
Buildroot:  /var/tmp/%{name}-%{version}-buildroot

#------------------------------------------------
# BASIC DEFINITIONS
#------------------------------------------------
# THIS SHOULD BE CHANGED
# %include rpm-dir.inc
%define _topdir %(echo $HOME)/rpmbuild

%define debug_package %{nil}
%define system	linux
%define APPS    /opt/apps
%define MODULES modulefiles

Summary: An Easy-to-Use Automatic Performance Diagnosis and Optimization Tool for HPC Applications
Group:   Applications/HPC

#------------------------------------------------
# PACKAGE DESCRIPTION
#------------------------------------------------
%description
PerfExpert (http://www.tacc.utexas.edu/perfexpert), an expert system built on
generations of performance measurement and analysis tools, utilizes knowledge
of architectures and compilers to implement (partial) automation of performance
optimization for multicore chips and heterogeneous nodes of cluster computers.

#------------------------------------------------
# INSTALLATION DIRECTORY
#------------------------------------------------
%define INSTALL_DIR %{APPS}/%{name}/%{version}
%define MODULE_DIR  %{APPS}/%{MODULES}/%{name}

#------------------------------------------------
# PREPARATION SECTION
#------------------------------------------------
%prep

# Fresh start
rm -rf $RPM_BUILD_ROOT

# Unpack source
%setup %{name}-%{version}

#------------------------------------------------
# BUILD SECTION
#------------------------------------------------
%build

# Start with a clean environment
if [ -f "$BASH_ENV" ]; then
  . $BASH_ENV
  module purge
  clearMT
  export MODULEPATH=/opt/apps/teragrid/modulefiles:/opt/apps/modulefiles:/opt/modulefiles
fi

module purge
module load TACC
module unload mvapich2 intel
module load gcc papi hpctoolkit

# Untar pre-requisites (need to de done before compiling)
tar -xzvf %{_topdir}/SOURCES/perfexpert-externals.tar.gz

#------------------------------------------------
# INSTALL SECTION
#------------------------------------------------
%install

# Configure PerfExpert
./autogen.sh
./configure --prefix=%{INSTALL_DIR} --with-externals=%{_topdir}/BUILD/%{name}-%{version}/externals --with-jvm=/usr/java/latest/jre/lib/amd64/server/ --with-papi=$TACC_PAPI_DIR --disable-hound --with-libxml2-include=/usr/include/libxml2/

# Compile and install PerfExpert
LD_LIBRARY_PATH=%{_topdir}/BUILD/%{name}-%{version}/externals/lib:$LD_LIBRARY_PATH make DESTDIR=$RPM_BUILD_ROOT install

# Copy pre-requisite libraries (no heaers required)
cp -r %{_topdir}/BUILD/%{name}-%{version}/externals/lib/* $RPM_BUILD_ROOT%{INSTALL_DIR}/lib/

# Copy Stampede-specific characterization files (required because hound was disabled)
cp contrib/stampede/*.conf $RPM_BUILD_ROOT%{INSTALL_DIR}/etc

# Add all the module stuff here
mkdir -p $RPM_BUILD_ROOT/%{MODULE_DIR}
cat > $RPM_BUILD_ROOT/%{MODULE_DIR}/%{version}.lua << 'EOF'

local help_message = [[
The perfexpert module makes TACC_PERFEXPERT_DIR available, adds the PerfExpert
directory to your PATH, and adds PerfExpert library directories to your
LD_LIBRARY_PATH.

WARNING: This version of PerfExpert will try to change your source code if you
         use the "-s" or "-m" options. Make sure to have a full copy of your
         code before running PerfExpert if you use them.

A Quick Start Guide can be found at http://www.tacc.utexas.edu/perfexpert/
The complete User Manual is available at http://tacc.github.io/perfexpert/
	
Version %{version}
]]

whatis("PerfExpert")
whatis("Version: %{version}")
whatis("Category: application, hpc")
whatis("Description: An Easy-to-Use Automatic Performance Diagnosis and Optimization Tool for HPC Applications")
whatis("URL: http://www.tacc.utexas.edu/perfexpert/")

help(help_message,"\n")

if( mode() == "load" ) then
  LmodMessage("\nWARNING: This version of PerfExpert will try to change your source code if you\n         use the -s or -m options. Make sure to have a full copy of the source\n         code before running PerfExpert on it.\n")
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
prepend_path("LD_LIBRARY_PATH",pathJoin(pe_dir,"/lib"))
prepend_path("LD_LIBRARY_PATH","/usr/java/latest/jre/lib/amd64/server/")

EOF

# THIS SHOULD BE UNCOMMENTED
# %{SPEC_DIR}/checkModuleSyntax $RPM_BUILD_ROOT/%{MODULE_DIR}/%{version}.lua

#------------------------------------------------
# FILES SECTION
#------------------------------------------------
%files

# Define files permisions, user and group
%defattr(-,root,install)
%{INSTALL_DIR}
%{MODULE_DIR}

#------------------------------------------------
# POST AND CLEAN UP SECTIONS
#------------------------------------------------
%post
%clean

# Remove the installation files now that the RPM has been generated
rm -rf $RPM_BUILD_ROOT
