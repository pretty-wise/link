import { Component, OnInit, Input, OnChanges, SimpleChanges } from '@angular/core';
import { Plugin } from '../shared/plugin';
import { PluginService } from '../plugin.service';
import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'app-plugin-list',
  templateUrl: './plugin-list.component.html',
  styleUrls: ['./plugin-list.component.css']
})
export class PluginListComponent implements OnInit, OnChanges {
  constructor(
    private pluginService: PluginService) { }

  @Input() commands: string[] = [];
  plugins: Observable<Plugin[]>;
  monitors: Observable<Plugin[]>;

  ngOnInit() {
    this.plugins = this.pluginService.getPluginList();
    this.monitors = this.pluginService.getMonitorList();
  }

  ngOnChanges(changes: SimpleChanges) {
    this.plugins = this.pluginService.getPluginList();
    this.monitors = this.pluginService.getMonitorList();
  }
}
