class common::users::pipeline (
  $ensure,
) {
  user { 'pipeline':
    ensure     => $ensure,
    comment    => 'Deployment Pipeline User',
    groups     => 'wheel',
    home       => '/home/pipeline',
    shell      => '/bin/bash',
    managehome => true,
  }
  ssh_authorized_key { 'develenv@qacore01':
    ensure => $ensure,
    key    => 'AAAAB3NzaC1kc3MAAACBAOh30luxLc9orrRPpwNWeTh5PTgIxGPKgD8IZPTlSH4ielCkH9rWP4T2RTO1l/I2DcHjGjNZe7XNQ2wT8eHopcd9/hkl9uPijByhbZ3NyxIcaQ+frXMaEFlrk8Pc+UhyScGwAgpeNRRUvd/4ZpZVNAEibeagBqXerG56bEXP6Gv5AAAAFQCglZn1TQrNpWjHFx77V3KZaqJB6wAAAIEAgdxyL/LSoiiDmMw3lFZinBqNqTU6fnl595dkxa4l5AsY3c/iVh1qqYkfdrprjAlNGrghQRAyTXjWJid5cWZ2vOlKFRYsNNu0x4SmCwTXBWeM2zkX4BB+Euf7dQlaw9pRi5Q3z/liJLgKatoVIveo9g7DUnv47BEtRAbRvdEyfQkAAACBANmqUtWDqbM0wzzjgXmeN7RyIVt/nvtjVB3hGrbKfXAQYcMp34B/lbcvcthtKHcPR5+mOljcqJQhYTVhF4IEcJJeDFPb/A+p+yvqJnvm+mQas76ne+aHf0l0BxiwY8toMnlOjED3aiXx99kJjt3g5dPH9PMbla1Nblh5gp0mIWl9',
    type   => 'dsa',
    user   => 'pipeline',
  }
}
