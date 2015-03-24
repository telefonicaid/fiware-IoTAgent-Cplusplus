#
# Dumps a file with the project name, the content type of the installation
# and the versioning of the package. 
#
# Example:
#      project=srvdir
#      version=0.0.1
#      release=307.g7eccf25
#
class tdaf_base::version(
    $project      = undef,
    $content      = undef,
    $version_file = '/var/lib/tdaf-puppet.version',
    $package      = undef,
) 
{
    
    if $project and $content and $package {
        exec { 'project':
            command => "echo \"project=$project\" > $version_file",
        }

        exec { 'content':
            command => "echo \"content=$content\" >> $version_file",
        }

        exec { 'version':
            command => "rpm -q --queryformat 'version=%{VERSION}\n' $package >> $version_file",
        }

        exec { 'release':
            command => "rpm -q --queryformat 'release=%{RELEASE}\n' $package >> $version_file",
        }
        
        Exec['project'] -> Exec['content'] -> Exec['version'] -> Exec['release']
    }
    else {
        warning 'Class tdaf_base::version declared but $project and $content and $package are not defined'
    }
}
