class common::users::root {
  # You can use grub-crypt to get the hash
  user { 'root':
    password => '$6$PzPQ4fHEXe72o7Pm$b/6B7KkIjLneEZqgDkgLbZslgGWKb70eGaz/EhaRdM81gUGKUZx0LEHlwvkVCMSm3FSXLVNneQ6tC69VxcUqq0',
  }
}
