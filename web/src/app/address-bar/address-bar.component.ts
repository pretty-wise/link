import { Component, OnInit } from '@angular/core';
import { Plugin } from '../shared/plugin';
import { PluginService, CommandListResponse } from '../plugin.service';

@Component({
  selector: 'app-address-bar',
  templateUrl: './address-bar.component.html',
  styleUrls: ['./address-bar.component.css']
})
export class AddressBarComponent implements OnInit {
  defaultRoot: string = 'http://localhost:8080';
  currentRoot: string = '';
  activeRoot: string = '';
  pluginListCommands: string[] = [];

  commands: string[] = [];

  constructor(
    private pluginService: PluginService) { }

  ngOnInit() {
    this.clear();
    this.currentRoot = this.defaultRoot;
    this.getCommandList();
  }

  clear() {
    this.commands = [];
    this.pluginListCommands = [];
  }

  getCommandList() {
    this.clear();
    this.activeRoot = this.currentRoot;
    this.pluginService.getCommandList(this.activeRoot).subscribe(commands => {
      this.commands = commands;

      // find the /plugin-list commands
      var plistCommands = commands.filter(function(x) { return x.includes("/plugin-list")});

      // setting this variable will cause the site to fetch the plugin list
      this.pluginListCommands = plistCommands.map(i => this.currentRoot + i);
    });
  }
}
