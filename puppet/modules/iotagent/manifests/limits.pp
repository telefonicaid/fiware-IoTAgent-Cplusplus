class iotagent::limits{
  include limits
  limits::conf{"nofile_root_hard":
    type =>"hard",
    value=>10240,
  }
  limits::conf{"nofile_root_soft":
    value=>2048,
  }
  limits::conf{"nofile_mongodb_hard":
    domain=>"mongodb",
    type =>"hard",
    value=>10240,
  }
  limits::conf{"nofile_mongodb_soft":
    domain=>"mongodb",
    value=>2048,
  }
}
