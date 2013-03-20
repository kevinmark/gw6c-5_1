#!/bin/sh

########################################################
# ONLY USE THIS ON A NON-BUILT SOURCE TREE OR YOU WILL
# GET UNWANTED FILES IN THE RESULTING PACKAGE.
########################################################

# Tested on FreeBSD 6.0-RELEASE 
# Since there are different implementations of the core
# utilities used by the script, some things might fail
# on your platform. Try adjusting the 'basic' functions
# below.

# TODO
# Add error checking.
# Maybe softcode the paths under the working directory.

# Uncomment the line below to help debugging.
# set -x

########################################################
# Global settings that you should take a look at before
# you try to do anything with this script. If you're
# just executing the script from a fresh CVS checkout,
# everything should be ok as is. 
########################################################

# The CVS base directory. This is the directory that contains
# the working directory from which you would like to create
# the package.
# 
CVS_BASE="../.."

# The working directory. This is located under the CVS_BASE
# directory, and is created when you issue a cvs checkout
# command such as:
# cvs checkout -d WORKING_DIR module
#
# WORKING_DIR will be the root of the generated archive,
# so make sure you issue the checkout with the name you
# want, and make sure the variable below matches it.
#
WORKING_DIR="tspc-advanced"

# The name of the archive to create. You can prepend a path
# if you don't want it in the directory where this script
# is located.
#
ARCHIVE_NAME="gw6c-4.2.2-src.tgz"

# The name of the staging directory that is created. The
# files are copied from the working directory to the staging
# directory before modifications are applied. The archive
# is created from the staging directory.
# 
STAGING_DIR="staging"

# The name of the directory that contains this script. As this
# is based on the directory structure in CVS, you shouldn't
# have to change it.
#
PACKAGE_DIR="package"


########################################################
# Basic functions you can redefine to fit the platform.
########################################################

# A function to output a string to standard output.
#
# $1 = String to output.
#
echo_message()
{
	echo "$1"
}

# A function to create a directory.
#
# $1 = Name of the directory to create.
#
create_directory()
{
	mkdir "$1"
}

# A function to remove a file or a directory.
#
# $1 = Name of the file or directory to remove.
#
remove_file()
{
	rm -rf "$1"
}

# A function to remove all directories that have a given name.
#
# $1 = Root of the file hierarchy. 
# $2 = Name of the directories to remove. 
#
remove_named_directories()
{
	find -d "$1" -name "$2" -type d -exec rm -rf {} \;
}

# A function to execute a sed script in-place on a file.
#
# $1 = sed script to execute.
# $2 = Name of the file to execute the sed script on (in-place).
sed_script()
{
	sed -i '' -e "$1" "$2"
}

# A function to create a gzip'ed tar file from the staging directory.
#
# $1 = Name and path of the archive to create.
# $2 = Name of the staging directory.
# $3 = Name of the CVS working directory.
#
archive_staging_directory()
{
	tar cvzf "$1" -C "$2" "$3"
}

# A function to copy files from the working directory to the staging directory.
# This means that, given:
# 
# $1 = Root of the CVS directory (where 'cvs co' was issued).
# $2 = Name of the CVS working directory.
# $3 = Name of the directory that contains the packaging scripts.
# $4 = Name of the staging directory.
#
# copy everything that is under $1/$2 to $4/$2, except if it is named $3.
# The exception is there because we don't want the packaging stuff to
# be included in the archive.
copy_to_staging_directory()
{
	create_directory $4/$2

	find $1/$2 -depth 1 ! -name $3 -exec cp -R {} $4/$2 \; 
}


########################################################
# Bigger functions that perform coarser subtasks.
########################################################

# A function to create the staging directory.
#
create_staging_directory()
{
	create_directory $STAGING_DIR

	copy_to_staging_directory $CVS_BASE $WORKING_DIR $PACKAGE_DIR $STAGING_DIR
}

# A function to remove the staging directory.
#
remove_staging_directory()
{
	remove_file $STAGING_DIR
}

# A function to remove the CVS specific stuff before we begin.
#
cleanup_cvs()
{
	echo_message "Removing CVS directories."
	remove_named_directories $STAGING_DIR "CVS"
}	

# A function to remove unused files and directories.
#
cleanup_unused()
{
	echo_message "Removing empty $STAGING_DIR/$WORKING_DIR/cm directory."
	remove_file $STAGING_DIR/$WORKING_DIR/cm

	echo_message "Removing empty $STAGING_DIR/$WORKING_DIR/src/tun directory."
	remove_file $STAGING_DIR/$WORKING_DIR/src/tun 
}

# A function to remove the Windows stuff from the Unix archive.
#
cleanup_windows()
{
	echo_message "Removing Windows-specific code."
	remove_file $STAGING_DIR/$WORKING_DIR/platform/windows

	echo_message "Removing Windows platform from Makefile choices."
	sed_script '/windows/d' $STAGING_DIR/$WORKING_DIR/Makefile 

	echo_message "Removing Windows template."
	remove_file $STAGING_DIR/$WORKING_DIR/template/windows.bat

	echo_message "Removing Windows version of the Cisco template."
	remove_file $STAGING_DIR/$WORKING_DIR/template/cisco.bat

	echo_message "Removing checktunnel.bat file."
	remove_file $STAGING_DIR/$WORKING_DIR/template/checktunnel.bat

	echo_message "Removing Windows references from the template README file."
	sed_script '/Windows/d' $STAGING_DIR/$WORKING_DIR/template/README

	echo_message "Removing Windows Makefile include (mk-windows.mk)."
	remove_file $STAGING_DIR/$WORKING_DIR/Mk/mk-windows.mk
}

# A function to remove the broken Solaris stuff.
#
cleanup_solaris()
{
	echo_message "Removing Solaris-specific code."
	remove_file $STAGING_DIR/$WORKING_DIR/platform/solaris

	echo_message "Removing Solaris platform from the Makefile choices."
	sed_script '/solaris/d' $STAGING_DIR/$WORKING_DIR/Makefile 

	echo_message "Removing Solaris template."
	remove_file $STAGING_DIR/$WORKING_DIR/template/solaris.sh

	echo_message "Removing Solaris Makefile include (mk-solaris.mk)."
	remove_file $STAGING_DIR/$WORKING_DIR/Mk/mk-solaris.mk

	echo_message "Removing Solaris from the 'gw6c.8' man page."
	sed_script 's/Solaris, //g' $STAGING_DIR/$WORKING_DIR/man/man8/gw6c.8

	echo_message "Removing Solaris from the possible 'template' values in 'gw6c.conf.in'."
	sed_script 's/solaris|//g' $STAGING_DIR/$WORKING_DIR/conf/gw6c.conf.in
}

# A function to create an archive from what's left.
#
create_archive()
{
	echo_message "Creating archive $ARCHIVE_NAME."
	archive_staging_directory $ARCHIVE_NAME $STAGING_DIR $WORKING_DIR	
}


########################################################
# Some error checking stuff. 
########################################################

# Just a quick check to see if the working directory is
# clean (non-built). There is a warning at the top of
# the file, but you never know.
#
verify_clean()
{
	if	test -d $CVS_BASE/$WORKING_DIR/objs || \
		test -d $CVS_BASE/$WORKING_DIR/bin
	then
		echo_message "Your working directory looks unclean!"
		echo_message "Read the warning at the top of the file."

		exit 1
	fi
}

########################################################
# Do what has to be done to package the thing. 
########################################################
package()
{
	echo_message "[ Packaging starting ]"

	remove_staging_directory
	create_staging_directory

	cleanup_cvs
	cleanup_unused
	cleanup_windows
	cleanup_solaris
	create_archive

	remove_staging_directory

	echo_message "[ Packaging done ]"
}


########################################################
# Fire it up. 
########################################################
verify_clean
package 

