# gyazo-server

## Synopsis

* gyazo-serverの構築をします
 * Ansibleでprovisioningします
* データ引き継ぎとserver移行作業で利用します
* 現状、各gem等の最新は追っていません

## Requirements

* ansible

## Notice

* httpで立ち上がります
* https対応方法
 1. 移行先domainの証明書でAnsibleに追加してください (ansible-vault推奨)
    Ansibleにnginxなど少しの変更で対応できます
 2. provisioning/group_vars/all の url: を http から https に変更します

## Prepare

* Creation of inventory is required.
* インベントリファイルを作成してください

## Installation

### Command Reference

* {{}} = Your file name.

 ```bash
% cd gyazo-server/
% ansible-playbook -i {{inventory file}} provisioning/init.yml
% ansible-playbook -i {{inventory file}} provisioning/gyazo.yml
```

### Use Client

* See
  - gyazo-server/clients/README.md
  - gyazo-server/clients/Win/readme.txt

* Mac
  - gyazo-server/clients/MacOSX

* Win
  - gyazo-server/clients/Win

## Test

### Vagrant

* prepare
 - Vagrantで確認する場合は /etc/hosts などで以下を書いておきます
  (group_vars/allで uri を動的に取得しているため)

 ```/etc/hosts
10.5.5.100 gyazo
```

1. vagrant up

 ```bash
% cd gyazo-server/
% vagrant up
```

2. Use clients/MacOSX/http-for-test-Gyazo.app


### Serverspec

 ```bash
% cat >> ~/.ssh/config << 'EOF'
Host gyazo
  HostName <Your gyazo server>
  UserKnownHostsFile /dev/null
  StrictHostKeyChecking no
  PasswordAuthentication no
  IdentityFile <Your private_key>
  IdentitiesOnly yes
  LogLevel FATAL
EOF
```

 ```bash
% cd gyazo-server/
% gem install serverspec --no-ri --no-rdoc
# bundler を使うならそれで
% rake serverspec:gyazo
```

## License

* Gyazoに準ずる
 - [Creative Commons License Japan](https://creativecommons.org/licenses/by-nc/2.1/jp/)
