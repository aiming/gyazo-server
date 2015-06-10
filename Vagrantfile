# -*- mode: ruby -*-
# vi: set ft=ruby :

require 'fileutils'

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"
CONFIG= "config.rb"

@vm_name = "gyazo"
@ip = "10.5.5.100"

# Vbox vm setting {
$vb_gui = false
$vb_memory = 512
$vb_cpus = 1
# }

if File.exist?(CONFIG)
  require_relative CONFIG
end

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  config.vm.provider :virtualbox do |vb|
    vb.gui = $vb_gui
    vb.memory = $vb_memory
    vb.cpus = $vb_cpus
  end

  # plugin conflict
  if Vagrant.has_plugin?("vagrant-vbguest") then
    config.vbguest.auto_update = false
  end

  node_name = @vm_name

  config.vm.define node_name do |server|
    server.vm.hostname = node_name
    server.vm.box = "hfm4/centos7"
    server.vm.network :private_network, ip: @ip
  end

  def exec_playbook(book, config)
    config.vm.provision "ansible" do |ansible|
      ansible.playbook = "provisioning/#{book}.yml"
      ansible.verbose = "vvvv"
      case book
      when "init"
        #ansible.skip_tags = ["yum_updates", "yum_repos", "selinux"]
        #ansible.skip_tags = ["yum_updates"]
      when "gyazo"
        #ansible.skip_tags = ["gem", "rbenv", "setup_rbenv"]
        #ansible.skip_tags = ["setup_rbenv"]
      #else
      #  break
      end

      #ansible.raw_arguments = ['--syntax-check']
    end
  end

  ['init', 'gyazo'].each {|p| exec_playbook(p, config)}

end

