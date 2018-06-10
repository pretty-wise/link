import { Component, OnInit } from '@angular/core';
import { Plugin } from '../shared/plugin';
import { PluginService, CommandListResponse } from '../plugin.service';
import { Observable } from 'rxjs';

@Component({
  selector: 'app-address-bar',
  templateUrl: './address-bar.component.html',
  styleUrls: ['./address-bar.component.css']
})
export class AddressBarComponent implements OnInit {
  defaultRoot: string = 'http://localhost:8080';
  currentRoot: string = '';
  activeRoot: string = '';

  commands: Observable<string[]>;

  constructor(
    private pluginService: PluginService) { }

  ngOnInit() {
    this.currentRoot = this.defaultRoot;
    this.getCommandList();
  }

  getCommandList() {
    this.activeRoot = this.currentRoot;

    this.pluginService.refresh(this.activeRoot);
    this.commands = this.pluginService.getCommandList();
  }
}
