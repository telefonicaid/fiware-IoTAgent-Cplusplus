# puppet-concatfile

## Description

A very simple Puppet module that concatenates multiple text parts together to one file. The single parts can be file sources, string content or existing files on the target system.

## Dependencies

* Core utilities: <code>find, sort, xargs</code>
* <code>dirname</code> function, e.g. as provided by http://github.com/puzzle/puppet-common 
* Tested on Puppet 0.25.4

## Usage

First, define the file that should be concatenated from the parts:

<pre>
<code>
concatfile {
  "/etc/sysctl.conf":
    dir => "/etc/sysctl.conf.d";
}
</code>
</pre>

The <code>dir</code> parameter specifies in which folder the parts are collected.

Then, define some parts:

<pre>
<code>
concatfile::part {
  "/etc/sysctl.conf.d/00.header":
    source => "puppet:///modules/sysctl/header";
  "/etc/sysctl.conf.d/01.sound":
    content => "# Default sound device\\nhw.snd.default_unit=1\\n\\n";
  "/etc/sysctl.conf.d/02.local":
    file => "/etc/sysctl.conf.local";
}
</code>
</pre>

The directory in the resource name of each part must match the <code>dir</code> parameter used in the <code>concatfile</code> resource, that is how Puppet knows they are connected. The files will later be sorted numerically and concatenated in that order.

The result is that the file <code>/etc/sysctl.conf</code> is concatenated from the 3 files in <code>/etc/sysctl.conf.d/</code>. Their content comes from 3 different possible sources: a Puppet files resource, a string and a symlink to a local file.

## Details

### Behavior

Each part is deployed as file in the specified folder. A simple exec command in the form of <code>find ... | sort ... | cat ... > target_file</code> is then used to concatenate all parts together. Apart from the basic update behavior from Puppet when any file is removed/modified, or any definition in the code is added/modified/removed, an update should also be triggered when a alien file is placed in the folder where the parts are stored, and the file gets removed.

### Parameters

Possible parameters to <code>concatfile</code> are:

* dir (required)
* owner
* group
* mode

Possible parameters to <code>concatfile::part</code> are:

* source
* content
* file
* owner
* group
* mode

Only one of source, content or file makes sense and is allowed.

### Extending

The defines can easily be wrapped to save some typing or e.g. introduce an order parameter:

<pre>
<code>
define sysctl::config(
  $content = "",
  $source = "",
  $order = "50"
) {

  concatfile::part {
    "/etc/sysctl.conf.d/$order.$name":
      content => $content,
      source => $source;
  }

}

sysctl::config {
  "header":
    source => "puppet:///modules/sysctl/header";
    order => "00";
}
</code>
</pre>

## Todo

* Triggering the update when the concatenated file is modified was not tested (did not have a puppet master around, and remembering checksums does not work completely with Puppet in standalone mode).


## Written By

Anselm Strauss http://github.com/amsibamsi
